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

#define SETTINGS_KEY_DBPATH "dbpath"
#define SETTINGS_KEY_FIRSTRUN "firstrun"
#define SETTINGS_KEY_IPCSOCKETPATH "ipcsocketpath"
#define SETTINGS_KEY_PDFVIEWER "pdfviewer"
#define SETTINGS_KEY_EXCLUDEDPATHS "excludedpaths"

inline void initResources()
{
	Q_INIT_RESOURCE(migrations);
}

bool Common::initSqliteDatabase(QString path)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(path);
	if(!db.open())
	{
		qDebug() << "failed to open database: " << path;
		return false;
	}
	initResources();
	DBMigrator migrator{db};
	migrator.performMigrations();
	db.close();
	return true;
}

QString Common::findInPath(QString needle)
{
	QStringList results;
	QString pathVar = QProcessEnvironment::systemEnvironment().value("PATH", "/usr/bin/:/bin/:");
	QStringList paths = pathVar.split(":");
	for(const QString &path : paths)
	{
		// TODO: can pass ../ but so be it for now.

		QFileInfo info{path + "/" + needle};
		if(info.exists())
		{
			return info.absoluteFilePath();
		}
	}
	return "";
}

void Common::setPdfViewer()
{
	QString value;

	/* TODO: well, we should query this probably from xdg*/
	QString okularPath = findInPath("okular");
	QString evincePath = findInPath("evince");
	QString qpdfviewPath = findInPath("qpdfview");

	if(okularPath != "")
	{
		value = okularPath + " %f -p %p";
	}
	else if(evincePath != "")
	{
		value = evincePath + "-i %p %f";
	}
	else if(qpdfviewPath != "")
	{
		value = qpdfviewPath + "%f#%p";
	}

	QSettings settings;
	if(value != "")
	{
		settings.setValue(SETTINGS_KEY_PDFVIEWER, value);
	}
}
void Common::ensureConfigured()
{
	QSettings settings;
	QVariant firstRun = settings.value(SETTINGS_KEY_FIRSTRUN);
	if(!firstRun.isValid())
	{
		QString dbpath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
		QDir dir;
		if(!dir.exists(dbpath))
		{
			if(!dir.mkpath(dbpath))
			{
				throw LooqsGeneralException("Failed to create dbpath directory");
			}
		}
		dbpath += "/looqs.sqlite";
		if(!initSqliteDatabase(dbpath))
		{
			throw LooqsGeneralException("Failed to initialize sqlite database");
		}

		settings.setValue(SETTINGS_KEY_FIRSTRUN, false);
		settings.setValue(SETTINGS_KEY_DBPATH, dbpath);
		setPdfViewer();
	}
	else
	{

		QString dbpath = databasePath();
		if(!QFile::exists(dbpath))
		{
			throw LooqsGeneralException("Database " + dbpath + " was not found");
		}
		DatabaseFactory factory{dbpath};
		auto db = factory.forCurrentThread();
		DBMigrator migrator{db};
		if(migrator.migrationNeeded())
		{
			QFile out;
			out.open(stderr, QIODevice::WriteOnly);
			Logger migrationLogger{&out};
			migrationLogger << "Database is being upgraded, please be patient..." << Qt::endl;
			QObject::connect(&migrator, &DBMigrator::migrationDone,
							 [&migrationLogger](uint32_t migration)
							 { migrationLogger << "Progress: Successfully migrated to: " << migration << Qt::endl; });
			migrator.performMigrations();
			migrationLogger << "Database upgraded successfully" << Qt::endl;
		}
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

QString Common::ipcSocketPath()
{
	return "/tmp/.looqs/looqs-ipc-socket";

	/* May not a good idea to set it in the settings and probably nobody would ever bother to change it anyway */
	// QSettings settings;
	// return settings.value(SETTINGS_KEY_IPCSOCKETPATH, "/tmp/.looqs/looqs-ipc-socket").toString();
}

static QStringList excludedPaths = {"/proc", "/sys", "/dev", "/tmp", "/var/run", "/run"};

QStringList Common::excludedPaths()
{
	static int ran = false;
	if(!ran)
	{
		QSettings settings;
		QStringList userExcludedPaths = settings.value(SETTINGS_KEY_EXCLUDEDPATHS).toStringList();
		ran = true;
		::excludedPaths.append(userExcludedPaths);
	}
	return ::excludedPaths;
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
