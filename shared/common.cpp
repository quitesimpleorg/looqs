#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QDebug>
#include <QMimeDatabase>
#include "looqsgeneralexception.h"
#include "common.h"
#include "dbmigrator.h"
#include "databasefactory.h"
#include "logger.h"

inline void initResources()
{
	Q_INIT_RESOURCE(migrations);
	Q_INIT_RESOURCE(plaintexts);
}

bool Common::initSqliteDatabase(QString path)
{
	try
	{
		initResources();
		DatabaseFactory factory(path);
		DBMigrator migrator{factory};
		migrator.performMigrations();
	}
	catch(std::exception &ex)
	{
		Logger::error() << "Failed to init dabase: " << ex.what();
		return false;
	}
	return true;
}

void Common::setPdfViewer()
{
	QString value;

	/* TODO: well, we should query this probably from xdg*/
	QString okularPath = QStandardPaths::findExecutable("okular");
	QString evincePath = QStandardPaths::findExecutable("evince");
	QString qpdfviewPath = QStandardPaths::findExecutable("qpdfview");

	if(okularPath != "")
	{
		value = okularPath + " %f -p %p";
	}
	else if(evincePath != "")
	{
		value = evincePath + " -i %p %f";
	}
	else if(qpdfviewPath != "")
	{
		value = qpdfviewPath + " %f#%p";
	}

	if(value != "")
	{
		QSettings settings;
		settings.setValue(SETTINGS_KEY_PDFVIEWER, value);
	}
}
void Common::ensureConfigured()
{
	QSettings settings;
	QString dbpath = databasePath();
	if(dbpath == "")
	{
		dbpath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
		QDir dir;
		if(!dir.exists(dbpath))
		{
			if(!dir.mkpath(dbpath))
			{
				throw LooqsGeneralException("Failed to create dbpath directory");
			}
		}
		dbpath += "/looqs.sqlite";
	}
	if(!QFile::exists(dbpath))
	{
		if(!initSqliteDatabase(dbpath))
		{
			throw LooqsGeneralException("Failed to initialize sqlite database");
		}
		settings.setValue(SETTINGS_KEY_DBPATH, dbpath);
	}
	QVariant pdfViewer = settings.value(SETTINGS_KEY_PDFVIEWER);
	if(!pdfViewer.isValid())
	{
		setPdfViewer();
	}
}

void Common::setupAppInfo()
{
	QCoreApplication::setOrganizationName("quitesimple.org");
	QCoreApplication::setOrganizationDomain("quitesimple.org");
	QCoreApplication::setApplicationName("looqs");
}

QString Common::databasePath()
{
	QString env = QProcessEnvironment::systemEnvironment().value("LOOQS_DB_OVERRIDE");
	if(env == "")
	{
		QSettings settings;
		return settings.value(SETTINGS_KEY_DBPATH).toString();
	}
	return env;
}

bool Common::noSandboxModeRequested()
{
	QString env = getenv("LOOQS_DISABLE_SANDBOX");
	if(env == "1")
	{
		return true;
	}
	return false;
}

QString Common::ipcSocketPath()
{
	return "/tmp/.looqs/looqs-ipc-socket";

	/* May not a good idea to set it in the settings and probably nobody would ever bother to change it anyway */
	// QSettings settings;
	// return settings.value(SETTINGS_KEY_IPCSOCKETPATH, "/tmp/.looqs/looqs-ipc-socket").toString();
}

QStringList Common::excludedPaths()
{
	static int ran = false;
	static QStringList excludedPaths;
	if(!ran)
	{
		QSettings settings;
		QStringList defaults{"/proc", "/sys", "/dev", "/tmp", "/var/run", "/run"};
		excludedPaths = settings.value(SETTINGS_KEY_EXCLUDEDPATHS, defaults).toStringList();
		ran = true;
	}
	return excludedPaths;
}

QStringList Common::mountPaths()
{
	static int ran = false;
	static QStringList mountPaths;
	if(!ran)
	{
		QSettings settings;
		mountPaths = settings.value(SETTINGS_KEY_MOUNTPATHS, QStringList{"/media", "/mnt"}).toStringList();
		ran = true;
	}
	return mountPaths;
}

bool Common::isMountPath(QString path)
{
	QStringList mountPaths = Common::mountPaths();
	for(QString &mountPath : mountPaths)
	{
		if(path.startsWith(mountPath))
		{
			return true;
		}
	}
	return false;
}

bool Common::isTextFile(QFileInfo fileInfo)
{
	/* TODO: This is not sandboxed yet ... */
	QMimeDatabase mimeDatabase;
	QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileInfo);
	if(mimeType.name().startsWith("text/"))
	{
		return true;
	}
	else
	{
		for(QString &str : mimeType.allAncestors())
		{
			if(str.startsWith("text/"))
			{
				return true;
			}
		}
	}
	return false;
}

QString Common::versionText()
{
	QString commitid = GIT_COMMIT_ID;
	QString tag = GIT_TAG;
	return tag + " (" + commitid + ") built " + __DATE__ + " " + __TIME__;
}

QString Common::userManualUrl()
{
	return QString("https://github.com/quitesimpleorg/looqs/blob/%1/USAGE.md").arg(QString(GIT_COMMIT_ID));
}
