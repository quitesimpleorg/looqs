#ifndef SQLITEDBSERVICE_H
#define SQLITEDBSERVICE_H
#include <QFileInfo>
#include <QMutexLocker>
#include <optional>

#include "databasefactory.h"
#include "utils.h"
#include "documentprocessresult.h"
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

	bool beginTransaction(QSqlDatabase &db);
	bool commitTransaction(QSqlDatabase &db);

	QMutex writeMutex;

  public:
	SqliteDbService(DatabaseFactory &dbFactory);
	SaveFileResult saveFile(QFileInfo fileInfo, DocumentProcessResult &pageData, bool pathsOnly);

	bool deleteFile(QString path);
	bool fileExistsInDatabase(QString path);
	bool fileExistsInDatabase(QString path, qint64 mtime);
	bool fileExistsInDatabase(QString path, qint64 mtime, QChar filetype);
	unsigned int getFiles(QVector<FileData> &results, QString wildCardPattern, int offset, int limit);

	bool addTag(QString tag, QString path);
	bool addTag(QString tag, const QVector<QString> &paths);
	QVector<QString> getTags();
	QVector<QString> getTagsForPath(QString path);
	QVector<QString> getPathsForTag(QString path);
	bool setTags(QString path, const QSet<QString> &tags);
	bool removePathsForTag(QString tag, const QVector<QString> &paths);
	bool deleteTag(QString tag);

	QVector<SearchResult> search(const LooqsQuery &query);

	std::optional<QChar> queryFileType(QString absPath);
	bool insertOutline(QSqlDatabase &db, int fileid, const QVector<DocumentOutlineEntry> &outlines);

	bool runWalCheckpoint();
};

#endif // SQLITEDBSERVICE_H
