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

	std::string sockPath = socketPath.toStdString();
	policy->namespace_options = EXILE_UNSHARE_NETWORK | EXILE_UNSHARE_USER;
	policy->vow_promises = EXILE_SYSCALL_VOW_THREAD | EXILE_SYSCALL_VOW_CPATH | EXILE_SYSCALL_VOW_WPATH |
						   EXILE_SYSCALL_VOW_RPATH | EXILE_SYSCALL_VOW_UNIX | EXILE_SYSCALL_VOW_STDIO |
						   EXILE_SYSCALL_VOW_PROT_EXEC | EXILE_SYSCALL_VOW_PROC | EXILE_SYSCALL_VOW_SHM |
						   EXILE_SYSCALL_VOW_FSNOTIFY | EXILE_SYSCALL_VOW_IOCTL;

	if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_REMOVE_FILE, "/") != 0)
	{
		qCritical() << "Failed to append a path to the path policy";
		exit(EXIT_FAILURE);
	}

	if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE,
								  appDataLocation.c_str()) != 0)
	{
		qCritical() << "Failed to append a path to the path policy";
		exit(EXIT_FAILURE);
	}
	if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE,
								  cacheDataLocation.c_str()) != 0)
	{
		qCritical() << "Failed to append a path to the path policy";
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

	if(!parser.isSet("no-sandbox"))
	{
		enableSandbox(socketPath);
		qInfo() << "Sandbox: on";
	}
	else
	{
		qInfo() << "Sandbox: off";
	}
	// Keep this post sandbox, afterwards does not work (suspect due to threads, but unconfirmed)
	QApplication a(argc, argv);
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
	qRegisterMetaType<QVector<SearchResult>>("QVector<SearchResult>");
	qRegisterMetaType<QVector<PreviewResultPdf>>("QVector<PreviewResultPdf>");
	qRegisterMetaType<PreviewResultPdf>("PreviewResultPdf");

	IPCClient client{socketPath};
	MainWindow w{0, client};
	w.showMaximized();

	return a.exec();
}
