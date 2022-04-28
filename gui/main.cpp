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

void enableSandbox(QString socketPath)
{
	struct exile_policy *policy = exile_init_policy();
	if(policy == NULL)
	{
		qCritical() << "Failed to init policy for sandbox";
		exit(EXIT_FAILURE);
	}
	QDir dir;
	dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
	dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));

	std::string appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString();
	std::string cacheDataLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation).toStdString();
	std::string configDataLocation = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString();

	std::string sockPath = socketPath.toStdString();
	std::string dbPath = QFileInfo(Common::databasePath()).absolutePath().toStdString();
	std::string mySelf = QFileInfo("/proc/self/exe").symLinkTarget().toStdString();
	policy->namespace_options = EXILE_UNSHARE_USER;
	if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ, "/") != 0)
	{
		qCritical() << "Failed to append a path to the path policy";
		exit(EXIT_FAILURE);
	}

	if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE,
								  appDataLocation.c_str()) != 0)
	{
		qCritical() << "Failed to append appDataLocation path to the path policy";
		exit(EXIT_FAILURE);
	}
	if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE,
								  cacheDataLocation.c_str()) != 0)
	{
		qCritical() << "Failed to append cacheDataLocation path to the path policy";
		exit(EXIT_FAILURE);
	}
	if(exile_append_path_policies(policy,
								  EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_REMOVE_FILE | EXILE_FS_ALLOW_ALL_WRITE,
								  dbPath.c_str()) != 0)
	{
		qCritical() << "Failed to append dbPath path to the path policy";
		exit(EXIT_FAILURE);
	}
	if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_EXEC, mySelf.c_str(), "/lib64",
								  "/lib") != 0)
	{
		qCritical() << "Failed to append mySelf path to the path policy";
		exit(EXIT_FAILURE);
	}
	if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE,
								  configDataLocation.c_str()) != 0)
	{
		qCritical() << "Failed to append configDataLocation path to the path policy";
		exit(EXIT_FAILURE);
	}
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
	if(!process.startDetached("/proc/self/exe", args))
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
			enableSandbox(socketPath);
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
	// Keep this post sandbox, afterwards does not work (suspect due to threads, but unconfirmed)
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/icon.svg"));

	qRegisterMetaType<QVector<SearchResult>>("QVector<SearchResult>");
	qRegisterMetaType<QVector<PreviewResultPdf>>("QVector<PreviewResultPdf>");
	qRegisterMetaType<PreviewResultPdf>("PreviewResultPdf");
	qRegisterMetaType<FileScanResult>("FileScanResult");

	IPCClient client{socketPath};
	MainWindow w{0, client};
	w.showMaximized();

	return a.exec();
}
