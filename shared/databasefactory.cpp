#include <QThread>
#include <QSqlQuery>
#include "databasefactory.h"
#include "logger.h"
#include "looqsgeneralexception.h"
DatabaseFactory::DatabaseFactory(QString connectionString)
{
	this->connectionString = connectionString;
}
static QThreadStorage<QSqlDatabase> dbStore;

// TODO: not threadsafe
QSqlDatabase DatabaseFactory::createNew()
{
	static int counter = 0;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "LOOQS" + QString::number(counter++));
	db.setDatabaseName(this->connectionString);
	if(!db.open())
	{
		Logger::error() << "Failed to open the database: " << this->connectionString << Qt::endl;
		throw LooqsGeneralException("Failed to create open new connection");
	}
	return db;
}

QSqlDatabase DatabaseFactory::forCurrentThread()
{
	if(dbStore.hasLocalData())
	{
		return dbStore.localData();
	}
	QSqlDatabase db =
		QSqlDatabase::addDatabase("QSQLITE", "LOOQS" + QString::number((quint64)QThread::currentThread(), 16));
	db.setDatabaseName(this->connectionString);
	db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=30000");
	if(!db.open())
	{
		Logger::error() << "Failed to open the database: " << this->connectionString << Qt::endl;
		throw LooqsGeneralException("Failed to create open new connection");
	}
	QSqlQuery q(db);
	if(!q.exec("PRAGMA journal_mode=WAL;"))
	{
		Logger::error() << "Failed to set WAL mode: " << this->connectionString << Qt::endl;
		throw LooqsGeneralException("Failed to set WAL mode on sqlite database");
	}
	if(!q.exec("PRAGMA wal_autocheckpoint=250;"))
	{
		Logger::error() << "Failed to set WAL autocheckpoint: " << this->connectionString << Qt::endl;
		throw LooqsGeneralException("Failed to set WAL autocheckpoint on sqlite database");
	}
	dbStore.setLocalData(db);
	return db;
}
