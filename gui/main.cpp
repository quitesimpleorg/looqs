#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcess>

#include "mainwindow.h"
#include "searchresult.h"
#include "pdfpreview.h"
#include "../shared/common.h"
#include "../submodules/exile.h/exile.h"
#include "ipcserver.h"

int main(int argc, char *argv[])
{
	QString socketPath = "/tmp/looqs-spawner";
	if(argc > 1)
	{
		Common::setupAppInfo();
		QApplication a(argc, argv);
		QString arg = argv[1];
		if(arg == "ipc")
		{
			IpcServer *ipcserver = new IpcServer();
			qDebug() << "Launching ipc";
			if(!ipcserver->startSpawner(socketPath))
			{
				qCritical() << "Error failed to spawn";
				return 1;
			}
			qDebug() << "Launched";
		}
		return a.exec();
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

	struct exile_policy *policy = exile_init_policy();
	if(policy == NULL)
	{
		qCritical() << "Failed to init policy for sandbox";
		return 1;
	}
	std::string appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString();
	std::string cacheDataLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation).toStdString();
	std::string sockPath = socketPath.toStdString();
	policy->namespace_options = EXILE_UNSHARE_NETWORK | EXILE_UNSHARE_USER;
	policy->vow_promises = EXILE_SYSCALL_VOW_THREAD | EXILE_SYSCALL_VOW_CPATH | EXILE_SYSCALL_VOW_WPATH |
						   EXILE_SYSCALL_VOW_RPATH | EXILE_SYSCALL_VOW_UNIX | EXILE_SYSCALL_VOW_STDIO |
						   EXILE_SYSCALL_VOW_PROT_EXEC | EXILE_SYSCALL_VOW_PROC | EXILE_SYSCALL_VOW_SHM |
						   EXILE_SYSCALL_VOW_FSNOTIFY | EXILE_SYSCALL_VOW_IOCTL;

	if(exile_append_path_policy(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_REMOVE_FILE, "/") != 0)
	{
		qCritical() << "Failed to append a path to the path policy";
		return 1;
	}

	if(exile_append_path_policy(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE, appDataLocation.c_str()) !=
	   0)
	{
		qCritical() << "Failed to append a path to the path policy";
		return 1;
	}
	if(exile_append_path_policy(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE,
								cacheDataLocation.c_str()) != 0)
	{
		qCritical() << "Failed to append a path to the path policy";
		return 1;
	}
	int ret = exile_enable_policy(policy);
	if(ret != 0)
	{
		qDebug() << "Failed to establish sandbox";
		return 1;
	}
	exile_free_policy(policy);

	Common::setupAppInfo();
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
	qRegisterMetaType<QVector<PdfPreview>>("QVector<PdfPreview>");
	qRegisterMetaType<PdfPreview>("PdfPreview");

	IPCClient client{socketPath};
	MainWindow w{0, client};
	w.showMaximized();

	return a.exec();
}
