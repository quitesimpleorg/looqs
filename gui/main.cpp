#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcess>
#include <QDir>
#include <QCommandLineParser>
#include <QFileInfo>

#include "mainwindow.h"
#include "searchresult.h"
#include "previewresultpdf.h"
#include "../shared/common.h"
#include "../shared/sandboxedprocessor.h"
#include "../submodules/exile.h/exile.h"
#include "ipcserver.h"

void enableIpcSandbox()
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
	policy->vow_promises =
		exile_vows_from_str("thread cpath wpath rpath unix stdio prot_exec proc shm fsnotify ioctl error");

	QString ipcSocketPath = Common::ipcSocketPath();
	QFileInfo info{ipcSocketPath};
	QString ipcSocketPathDir = info.absolutePath();
	std::string stdIpcSocketPath = ipcSocketPathDir.toStdString();

	exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ, "/");
	exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE, stdIpcSocketPath.c_str());
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
	QString socketPath = Common::ipcSocketPath();
	if(argc > 1)
	{
		QString arg = argv[1];
		if(arg == "ipc")
		{
			Common::setupAppInfo();
			enableIpcSandbox();
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
			QCoreApplication a(argc, argv);

			QStringList args = a.arguments();
			if(args.length() < 3)
			{
				qDebug() << "Filename is required";
				return 1;
			}

			QString file = args.at(2);
			SandboxedProcessor processor(file);
			return processor.process();
		}
	}
	QString ipcSocketPath = Common::ipcSocketPath();
	QFileInfo info{ipcSocketPath};
	QString ipcSocketPathDir = info.absolutePath();

	QDir dir;
	if(!dir.mkpath(ipcSocketPathDir))
	{
		qCritical() << "Failed to create dir for ipc socket" << Qt::endl;
		exit(EXIT_FAILURE);
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
