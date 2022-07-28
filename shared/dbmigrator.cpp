#include <QDirIterator>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QDebug>
#include "dbmigrator.h"
#include "looqsgeneralexception.h"

DBMigrator::DBMigrator(DatabaseFactory &factory)
{
	Q_INIT_RESOURCE(migrations);
	this->dbFactory = &factory;
}

DBMigrator::~DBMigrator()
{
	Q_CLEANUP_RESOURCE(migrations);
}

QStringList DBMigrator::getMigrationFilenames()
{
	QStringList result;
	QDirIterator it(":/looqs-migrations/");
	while(it.hasNext())
	{
		result.append(it.next());
	}
	return result;
}

uint32_t DBMigrator::currentRevision()
{
	QSqlDatabase db = dbFactory->forCurrentThread();
	QSqlQuery dbquery(db);
	dbquery.exec("PRAGMA user_version;");
	if(!dbquery.next())
	{
		throw new LooqsGeneralException("Failed to query current db revision");
	}
	uint32_t result = dbquery.value(0).toUInt();
	return result;
}

bool DBMigrator::migrationNeeded()
{
	QStringList migrations = getMigrationFilenames();
	uint32_t currentRev = currentRevision();

	return currentRev < static_cast<uint32_t>(migrations.size());
}

void DBMigrator::start()
{
	performMigrations();
}

void DBMigrator::performMigrations()
{
	QStringList migrations = getMigrationFilenames();
	uint32_t currentRev = currentRevision();
	uint32_t targetRev = (migrations.size());

	QSqlDatabase db = dbFactory->forCurrentThread();

	try
	{
		for(uint32_t i = currentRev + 1; i <= targetRev; i++)
		{
			QString fileName = QString(":/looqs-migrations/%1.sql").arg(i);
			QFile file{fileName};
			if(!file.open(QIODevice::ReadOnly))
			{
				throw LooqsGeneralException("Migration: Failed to find required revision file");
			}
			QTextStream stream(&file);
			db.transaction();
			while(!stream.atEnd())
			{
				QString sql = stream.readLine();
				QSqlQuery sqlQuery{db};
				if(!sqlQuery.exec(sql))
				{

					db.rollback();
					throw LooqsGeneralException("Failed to execute sql statement while initializing database: " +
												sqlQuery.lastError().text());
				}
			}
			QSqlQuery updateVersion{db};
			updateVersion.exec(QString("PRAGMA user_version=%1;").arg(i));
			db.commit();

			QSqlQuery vacuumQuery{db};
			vacuumQuery.exec("VACUUM;");

			emit migrationDone(i);
		}
		emit done();
	}
	catch(LooqsGeneralException &e)
	{
		emit error(e.message);
	}
}
