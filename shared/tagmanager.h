#ifndef TAGMANAGER_H
#define TAGMANAGER_H
#include "sqlitedbservice.h"

class TagManager
{
  private:
	SqliteDbService *dbService = nullptr;
	bool ensurePathOkay(QString inpath);

  public:
	TagManager(SqliteDbService &dbService);

	bool addTagsToPath(QString path, const QSet<QString> &tags);
	bool addTagsToPath(QString path, QString tagstring, QChar delim);

	bool addPathsToTag(QString tag, const QVector<QString> &paths);
	bool removeTagsForPath(QString path, const QSet<QString> &tags);

	bool removePathsForTag(QString tag, const QVector<QString> &paths);
	bool deleteTag(QString tag);

	QVector<QString> getTags(QString path);
	QVector<QString> getTags();
	QVector<QString> getPaths(QString tag);
};

#endif // TAGMANAGER_H
