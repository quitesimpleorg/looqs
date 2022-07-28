#ifndef DBMIGRATOR_H
#define DBMIGRATOR_H
#include <QStringList>
#include <QSqlDatabase>
#include <QObject>
#include "databasefactory.h"
class DBMigrator : public QObject
{
	Q_OBJECT
  private:
	DatabaseFactory *dbFactory;

  public:
	DBMigrator(DatabaseFactory &dbFactory);
	~DBMigrator();
	uint32_t currentRevision();
	void performMigrations();
	QStringList getMigrationFilenames();
	bool migrationNeeded();
  signals:
	void migrationDone(uint32_t);
	void done();
	void error(QString e);
  public slots:
	void start();
};

#endif // DBMIGRATOR_H
