#ifndef SQLITEDBSERVICE_H
#define SQLITEDBSERVICE_H
#include <QFileInfo>
#include <optional>

#include "databasefactory.h"
#include "utils.h"
#include "pagedata.h"
#include "filedata.h"
#include "../shared/sqlitesearch.h"
#include "../shared/token.h"
#include "savefileresult.h"

class SqliteDbService
{
  private:
	DatabaseFactory *dbFactory = nullptr;
	bool insertToFTS(bool useTrigrams, QSqlDatabase &db, int fileid, QVector<PageData> &pageData);

	QSqlQuery exec(QString query, std::initializer_list<QVariant> args);
	bool execBool(QString querystr, std::initializer_list<QVariant> args);

  public:
	SqliteDbService(DatabaseFactory &dbFactory);
	SaveFileResult saveFile(QFileInfo fileInfo, QVector<PageData> &pageData, bool pathsOnly);
	unsigned int getFiles(QVector<FileData> &results, QString wildCardPattern, int offset, int limit);
	bool deleteFile(QString path);
	bool fileExistsInDatabase(QString path);
	bool fileExistsInDatabase(QString path, qint64 mtime);
	bool fileExistsInDatabase(QString path, qint64 mtime, QChar filetype);
	QVector<SearchResult> search(const LooqsQuery &query);

	std::optional<QChar> queryFileType(QString absPath);
};

#endif // SQLITEDBSERVICE_H
