#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QDebug>
#include "looqsgeneralexception.h"
#include "common.h"
#include "dbmigrator.h"
#include "databasefactory.h"
#include "logger.h"

#define SETTINGS_KEY_DBPATH "dbpath"
#define SETTINGS_KEY_FIRSTRUN "firstrun"
#define SETTINGS_KEY_IPCSOCKETPATH "ipcsocketpath"

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
	QString env = QProcessEnvironment::systemEnvironment().value("QSS_DB_OVERRIDE");
	if(env == "")
	{
		QSettings settings;
		return settings.value(SETTINGS_KEY_DBPATH).toString();
	}
	return env;
}

QString Common::ipcSocketPath()
{
	QSettings settings;
	return settings.value(SETTINGS_KEY_IPCSOCKETPATH, "/tmp/looqs-spawner").toString();
}
