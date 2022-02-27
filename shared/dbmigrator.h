#ifndef DBMIGRATOR_H
#define DBMIGRATOR_H
#include <QStringList>
#include <QSqlDatabase>
#include <QObject>
class DBMigrator : public QObject
{
	Q_OBJECT
  private:
	QSqlDatabase *db;

  public:
	DBMigrator(QSqlDatabase &db);
	~DBMigrator();
	uint32_t currentRevision();
	void performMigrations();
	QStringList getMigrationFilenames();
	bool migrationNeeded();
  signals:
	void migrationDone(uint32_t);
	void done();
};

#endif // DBMIGRATOR_H
