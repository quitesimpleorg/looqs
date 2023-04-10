#include "tagmanager.h"

TagManager::TagManager(SqliteDbService &dbService)
{
	this->dbService = &dbService;
}

bool TagManager::addTagsToPath(QString path, const QSet<QString> &tags)
{
	QVector<QString> currentTags = this->dbService->getTagsForPath(path);
	for(const QString &tag : tags)
	{
		currentTags.append(tag.toLower());
	}

	QSet<QString> newTags{currentTags.begin(), currentTags.end()};
	return this->dbService->setTags(path, newTags);
}

bool TagManager::removeTagsForPath(QString path, const QSet<QString> &tags)
{
	QVector<QString> currentTags = this->dbService->getTagsForPath(path);
	for(const QString &tag : tags)
	{
		currentTags.removeAll(tag);
	}
	QSet<QString> newTags{currentTags.begin(), currentTags.end()};
	return this->dbService->setTags(path, newTags);
}

bool TagManager::addTagsToPath(QString path, QString tagstring, QChar delim)
{
	auto splitted = tagstring.split(delim);

	return addTagsToPath(path, QSet<QString>{splitted.begin(), splitted.end()});
}

bool TagManager::addPathsToTag(QString tag, const QVector<QString> &paths)
{
	return this->dbService->addTag(tag, paths);
}
