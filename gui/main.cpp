#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcess>
#include <QDir>
#include <QCommandLineParser>

#include "mainwindow.h"
#include "searchresult.h"
#include "previewresultpdf.h"
#include "../shared/common.h"
#include "../shared/sandboxedprocessor.h"
#include "../submodules/exile.h/exile.h"
#include "ipcserver.h"

void enableSandbox()
{
	struct exile_policy *policy = exile_create_policy();
	if(policy == NULL)
	{
		qCritical() << "Failed to init policy for sandbox";
		exit(EXIT_FAILURE);
	}
	policy->namespace_options = 0;
	policy->no_new_privs = 1;
	int ret = exile_enable_policy(policy);
	if(ret != 0)
	{
		qDebug() << "Failed to establish sandbox";
		exit(EXIT_FAILURE);
	}
	exile_free_policy(policy);
}

void enableIpcSandbox(QString socketPath)
{
	struct exile_policy *policy = exile_create_policy();
	if(policy == NULL)
	{
		qCritical() << "Failed to init policy for sandbox";
		exit(EXIT_FAILURE);
	}
	policy->namespace_options = EXILE_UNSHARE_NETWORK | EXILE_UNSHARE_USER;
	policy->no_new_privs = 1;
	policy->drop_caps = 1;
	policy->vow_promises = exile_vows_from_str("thread cpath wpath rpath unix stdio prot_exec proc shm fsnotify ioctl");

	exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ, "/");
	exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE, "/tmp");
	int ret = exile_enable_policy(policy);
	if(ret != 0)
	{
		qDebug() << "Failed to establish sandbox";
		exit(EXIT_FAILURE);
	}
	exile_free_policy(policy);
}

int main(int argc, char *argv[])
{
	QString socketPath = "/tmp/looqs-spawner";
	if(argc > 1)
	{
		QString arg = argv[1];
		if(arg == "ipc")
		{
			Common::setupAppInfo();
			enableIpcSandbox(socketPath);
			QApplication a(argc, argv);

			IpcServer *ipcserver = new IpcServer();
			qDebug() << "Launching IPC Server";
			if(!ipcserver->startSpawner(socketPath))
			{
				qCritical() << "Error failed to spawn";
				return 1;
			}
			qDebug() << "Launched IPC Server";
			return a.exec();
		}
		if(arg == "process")
		{
			Common::setupAppInfo();
			QApplication a(argc, argv);

			QStringList args = a.arguments();
			if(args.length() < 1)
			{
				qDebug() << "Filename is required";
				return 1;
			}

			QString file = args.at(1);
			SandboxedProcessor processor(file);
			return processor.process();
		}
	}
	QProcess process;
	QStringList args;
	args << "ipc";
	process.setProcessChannelMode(QProcess::ForwardedChannels);

	process.start("/proc/self/exe", args);
	if(!process.waitForStarted(5000))
	{
		QString errorMsg = "Failed to start IPC server";
		qDebug() << errorMsg;
		QMessageBox::critical(nullptr, "Error", errorMsg);
	}
	Common::setupAppInfo();
	QCommandLineParser parser;
	parser.addOption({{"s", "no-sandbox"}, "Disable sandboxing"});
	QStringList appArgs;
	for(int i = 0; i < argc; i++)
	{
		appArgs.append(argv[i]);
	}
	parser.parse(appArgs);

	try
	{
		Common::ensureConfigured();
		if(!parser.isSet("no-sandbox"))
		{
			enableSandbox();
			qInfo() << "Sandbox: on";
		}
		else
		{
			qInfo() << "Sandbox: off";
		}
	}
	catch(LooqsGeneralException &e)
	{
		qDebug() << e.message;
		QMessageBox::critical(nullptr, "Error", e.message);
		return 1;
	}
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/icon.svg"));
	QObject::connect(&a, &QApplication::aboutToQuit, &process, &QProcess::kill);

	qRegisterMetaType<QVector<SearchResult>>("QVector<SearchResult>");
	qRegisterMetaType<QVector<PreviewResultPdf>>("QVector<PreviewResultPdf>");
	qRegisterMetaType<PreviewResultPdf>("PreviewResultPdf");
	qRegisterMetaType<FileScanResult>("FileScanResult");
	qRegisterMetaType<RenderConfig>("RenderConfig");
	qRegisterMetaType<QVector<RenderTarget>>("QVector<RenderTarget>");
	qRegisterMetaType<QSharedPointer<PreviewResult>>("QSharedPointer<PreviewResult>");
	MainWindow w{0, socketPath};
	w.showMaximized();
	return a.exec();
}
