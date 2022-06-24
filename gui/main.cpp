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
	policy->namespace_options = EXILE_UNSHARE_USER | EXILE_UNSHARE_MOUNT | EXILE_UNSHARE_NETWORK;
	policy->no_new_privs = 1;
	policy->drop_caps = 1;
	policy->vow_promises = exile_vows_from_str("thread cpath rpath unix stdio proc error");
	policy->mount_path_policies_to_chroot = 1;

	QString ipcSocketPath = Common::ipcSocketPath();
	QFileInfo info{ipcSocketPath};
	QString ipcSocketPathDir = info.absolutePath();
	std::string stdIpcSocketPath = ipcSocketPathDir.toStdString();

	/* we only need the 'server' side of the 'unix' vow (for unix sockets)'. The process
	 * has no business to connect anywhere.
	 *
	 * Maybe this case should be handled by exile at some point, but for now deal with it here */
	exile_append_syscall_policy(policy, EXILE_SYS(connect), EXILE_SYSCALL_DENY_RET_ERROR, NULL, 0);

	/* ALLOW_EXEC is needed for fallback, not in landlock mode. It does not allow executing anything though here
	 * due to the vows */
	exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_EXEC, "/");
	exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ | EXILE_FS_ALLOW_ALL_WRITE | EXILE_FS_ALLOW_EXEC,
							   stdIpcSocketPath.c_str());
	int ret = exile_enable_policy(policy);
	if(ret != 0)
	{
		qDebug() << "Failed to establish sandbox" << Qt::endl;
		exit(EXIT_FAILURE);
	}

	/* Arguments are irrelevant for sandbox test, just want to silence analyzer/compiler warnings */
	ret = socket(AF_INET, SOCK_STREAM, 0);
	if(ret != -1 || errno != EACCES)
	{
		qCritical() << "Sandbox sanity check failed" << Qt::endl;
		exit(EXIT_FAILURE);
	}

	const struct sockaddr *addr = {};
	ret = connect(3, addr, sizeof(*addr));
	if(ret != -1 || errno != EACCES)
	{
		qCritical() << "Sandbox sanity check failed" << Qt::endl;
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
			if(Common::noSandboxModeRequested())
			{
				qInfo() << "Launching with no sandbox!" << Qt::endl;
			}
			else
			{
				enableIpcSandbox();
			}
			QCoreApplication a(argc, argv);

			IpcServer *ipcserver = new IpcServer();
			qDebug() << "Launching IPC Server";
			if(!ipcserver->startSpawner(socketPath))
			{
				qCritical() << "Error failed to spawn" << Qt::endl;
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
			return static_cast<int>(processor.process());
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
	QStringList appArgs;
	for(int i = 0; i < argc; i++)
	{
		appArgs.append(argv[i]);
	}
	parser.parse(appArgs);

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
	a.setWindowIcon(QIcon(":/looqs.svg"));
	QObject::connect(&a, &QApplication::aboutToQuit, &process, &QProcess::kill);

	qRegisterMetaType<QVector<SearchResult>>("QVector<SearchResult>");
	qRegisterMetaType<QVector<PreviewResultPdf>>("QVector<PreviewResultPdf>");
	qRegisterMetaType<PreviewResultPdf>("PreviewResultPdf");
	qRegisterMetaType<FileScanResult>("FileScanResult");
	qRegisterMetaType<RenderConfig>("RenderConfig");
	qRegisterMetaType<QVector<RenderTarget>>("QVector<RenderTarget>");
	qRegisterMetaType<QSharedPointer<PreviewResult>>("QSharedPointer<PreviewResult>");
	MainWindow *w = new MainWindow{0, socketPath};
	w->showMaximized();
	int ret = a.exec();
	process.waitForFinished(1000);
	return ret;
}
