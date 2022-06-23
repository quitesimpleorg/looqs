#ifndef SQLITEDBSERVICE_H
#define SQLITEDBSERVICE_H
#include <QFileInfo>
#include "databasefactory.h"
#include "utils.h"
#include "pagedata.h"
#include "filedata.h"
#include "../shared/sqlitesearch.h"
#include "../shared/token.h"
enum SaveFileResult
{
	OK,
	SKIPPED,
	DBFAIL,
	PROCESSFAIL,
	NOTFOUND,
	NOACCESS
};

static inline QString SaveFileResultToString(SaveFileResult sfr)
{
	QStringList SaveFileResultStr;

	SaveFileResultStr << "OK"
					  << "SKIPPED"
					  << "DBFAIL"
					  << "PROCESSFAIL"
					  << "NOTFOUND"
					  << "NOACCESS";

	return SaveFileResultStr[(int)sfr];
}

class SqliteDbService
{
  private:
	DatabaseFactory *dbFactory = nullptr;

  public:
	SqliteDbService(DatabaseFactory &dbFactory);
	SaveFileResult saveFile(QFileInfo fileInfo, QVector<PageData> &pageData);
	unsigned int getFiles(QVector<FileData> &results, QString wildCardPattern, int offset, int limit);
	bool deleteFile(QString path);
	bool fileExistsInDatabase(QString path);
	bool fileExistsInDatabase(QString path, qint64 mtime);
	QVector<SearchResult> search(const LooqsQuery &query);
};

#endif // SQLITEDBSERVICE_H
