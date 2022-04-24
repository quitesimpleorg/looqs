#include <QThread>
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
	if(!db.open())
	{
		Logger::error() << "Failed to open the database: " << this->connectionString << Qt::endl;
		throw LooqsGeneralException("Failed to create open new connection");
	}
	dbStore.setLocalData(db);
	return db;
}
