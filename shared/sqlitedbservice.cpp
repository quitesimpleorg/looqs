#include <QSqlQuery>
#include <QFileInfo>
#include <QDateTime>
#include <QSqlError>
#include <QRegularExpression>
#include "looqsgeneralexception.h"
#include "sqlitedbservice.h"
#include "filedata.h"
#include "logger.h"

QVector<SearchResult> SqliteDbService::search(const LooqsQuery &query)
{
	auto connection = dbFactory->forCurrentThread();
	SqliteSearch searcher(connection);
	return searcher.search(query);
}

std::optional<QChar> SqliteDbService::queryFileType(QString absPath)
{
	auto query = exec("SELECT filetype FROM file WHERE path = ?", {absPath});
	if(!query.next())
	{
		return {};
	}
	return query.value(0).toChar();
}

bool SqliteDbService::fileExistsInDatabase(QString path)
{
	return execBool("SELECT 1 FROM file WHERE path = ?", {path});
}

bool SqliteDbService::fileExistsInDatabase(QString path, qint64 mtime)
{
	return execBool("SELECT 1 FROM file WHERE path = ? AND mtime = ?", {path, mtime});
}

bool SqliteDbService::fileExistsInDatabase(QString path, qint64 mtime, QChar fileType)
{
	return execBool("SELECT 1 FROM file WHERE path = ? AND mtime = ? AND filetype = ?", {path, mtime, fileType});
}

SqliteDbService::SqliteDbService(DatabaseFactory &dbFactory)
{
	this->dbFactory = &dbFactory;
}

bool SqliteDbService::deleteFile(QString path)
{
	QSqlQuery query(this->dbFactory->forCurrentThread());
	query.prepare("DELETE FROM file WHERE path = ?");
	query.addBindValue(path);
	bool result = query.exec();
	if(!result)
	{
		Logger::error() << "Failed to delete file" << path << Qt::endl;
	}
	return result;
}

unsigned int SqliteDbService::getFiles(QVector<FileData> &results, QString wildCardPattern, int offset, int limit)
{

	unsigned int processedRows = 0;
	// TODO: translate/convert wildCardPattern to SQL where instead of regex
	QString sql = "SELECT path, mtime, size, filetype FROM file";

	if(limit != 0)
	{
		sql += " LIMIT " + QString::number(limit);
	}
	if(offset != 0)
	{
		sql += " OFFSET " + QString::number(offset);
	}

	auto query = QSqlQuery(dbFactory->forCurrentThread());
	query.prepare(sql);
	query.setForwardOnly(true);
	if(!query.exec())
	{
		throw LooqsGeneralException("Error while trying to retrieve files from database: " + query.lastError().text());
	}

	// TODO: port this to QRegularExpression once >5.12 gets more widespread because of this bug
	// https://bugreports.qt.io/browse/QTBUG-72539?focusedCommentId=439053&page=com.atlassian.jira.plugin.system.issuetabpanels%3Acomment-tabpanel
	bool usePattern = !wildCardPattern.isEmpty();
	QRegularExpression regexPattern(QRegularExpression::wildcardToRegularExpression(wildCardPattern));

	while(query.next())
	{
		QString absPath = query.value(0).toString();
		if(!usePattern || regexPattern.match(absPath).hasMatch())
		{
			FileData current;
			current.absPath = absPath;
			current.mtime = query.value(1).toInt();
			current.size = query.value(2).toInt();
			current.filetype = query.value(3).toChar();
			results.append(current);
		}
		++processedRows;
	}
	return processedRows;
}

QVector<QString> SqliteDbService::getTags()
{
	QVector<QString> result;
	auto query = QSqlQuery(dbFactory->forCurrentThread());
	query.prepare("SELECT name FROM tag ORDER by name ASC");
	query.setForwardOnly(true);
	if(!query.exec())
	{
		throw LooqsGeneralException("Error while trying to retrieve tags from database: " + query.lastError().text());
	}
	while(query.next())
	{
		QString tagname = query.value(0).toString();
		result.append(tagname);
	}
	return result;
}

QVector<QString> SqliteDbService::getTagsForPath(QString path)
{
	QVector<QString> result;
	auto query = QSqlQuery(dbFactory->forCurrentThread());
	query.prepare("SELECT name FROM tag INNER JOIN filetag ON tag.id = filetag.tagid INNER JOIN file ON filetag.fileid "
				  "= file.id WHERE file.path = ? ORDER BY name ASC");
	query.addBindValue(path);
	query.setForwardOnly(true);
	if(!query.exec())
	{
		throw LooqsGeneralException("Error while trying to retrieve tags from database: " + query.lastError().text());
	}
	while(query.next())
	{
		QString tagname = query.value(0).toString();
		result.append(tagname);
	}
	return result;
}

QVector<QString> SqliteDbService::getPathsForTag(QString tag)
{
	QVector<QString> result;
	auto query = QSqlQuery(dbFactory->forCurrentThread());
	query.prepare(
		"SELECT file.path FROM tag INNER JOIN filetag ON tag.id = filetag.tagid INNER JOIN file ON filetag.fileid "
		"= file.id WHERE tag.name = ?");
	query.addBindValue(tag.toLower());
	query.setForwardOnly(true);
	if(!query.exec())
	{
		throw LooqsGeneralException("Error while trying to retrieve paths from database: " + query.lastError().text());
	}
	while(query.next())
	{
		QString path = query.value(0).toString();
		result.append(path);
	}
	return result;
}

bool SqliteDbService::setTags(QString path, const QSet<QString> &tags)
{
	QSqlDatabase db = dbFactory->forCurrentThread();
	if(!db.transaction())
	{
		Logger::error() << "Failed to open transaction for " << path << " : " << db.lastError() << Qt::endl;
		return false;
	}

	QSqlQuery deletionQuery = QSqlQuery(db);
	deletionQuery.prepare("DELETE FROM filetag WHERE fileid = (SELECT id FROM file WHERE path = ?)");
	deletionQuery.addBindValue(path);
	if(!deletionQuery.exec())
	{
		db.rollback();
		Logger::error() << "Failed to delete existing tags " << deletionQuery.lastError() << Qt::endl;
		return false;
	}

	for(const QString &tag : tags)
	{
		QSqlQuery tagQuery = QSqlQuery(db);
		tagQuery.prepare("INSERT OR IGNORE INTO tag (name) VALUES(?)");
		tagQuery.addBindValue(tag.toLower());
		if(!tagQuery.exec())
		{
			db.rollback();
			Logger::error() << "Failed to insert tag " << tagQuery.lastError() << Qt::endl;
			return false;
		}
		QSqlQuery fileTagQuery(db);
		fileTagQuery.prepare(
			"INSERT INTO filetag(fileid, tagid) VALUES((SELECT id FROM file WHERE path = ?), (SELECT id "
			"FROM tag WHERE name = ?))");
		fileTagQuery.bindValue(0, path);
		fileTagQuery.bindValue(1, tag);
		if(!fileTagQuery.exec())
		{
			db.rollback();
			Logger::error() << "Failed to assign tag to file" << Qt::endl;
			return false;
		}
	}
	if(!db.commit())
	{
		db.rollback();
		Logger::error() << "Failed to commit transaction when saving tags" << Qt::endl;
		return false;
	}
	return true;
}

bool SqliteDbService::insertToFTS(bool useTrigrams, QSqlDatabase &db, int fileid, QVector<PageData> &pageData)
{
	QString ftsInsertStatement;
	QString contentInsertStatement;
	if(useTrigrams)
	{
		ftsInsertStatement = "INSERT INTO fts_trigram(content) VALUES(?)";
		contentInsertStatement = "INSERT INTO content(fileid, page, fts_trigramid) VALUES(?, ?, last_insert_rowid())";
	}
	else
	{
		ftsInsertStatement = "INSERT INTO fts(content) VALUES(?)";
		contentInsertStatement = "INSERT INTO content(fileid, page, ftsid) VALUES(?, ?, last_insert_rowid())";
	}

	for(const PageData &data : pageData)
	{
		QSqlQuery ftsQuery(db);
		ftsQuery.prepare(ftsInsertStatement);
		ftsQuery.addBindValue(data.content);
		if(!ftsQuery.exec())
		{
			Logger::error() << "Failed fts insertion " << ftsQuery.lastError() << Qt::endl;
			return false;
		}
		QSqlQuery contentQuery(db);
		contentQuery.prepare(contentInsertStatement);
		contentQuery.addBindValue(fileid);
		contentQuery.addBindValue(data.pagenumber);
		if(!contentQuery.exec())
		{
			Logger::error() << "Failed content insertion " << contentQuery.lastError() << Qt::endl;
			return false;
		}
	}
	return true;
}

bool SqliteDbService::insertOutline(QSqlDatabase &db, int fileid, const QVector<DocumentOutlineEntry> &outlines)
{
	QSqlQuery outlineQuery(db);
	outlineQuery.prepare("INSERT INTO outline(fileid, text, page) VALUES(?,?,?)");
	outlineQuery.addBindValue(fileid);
	for(const DocumentOutlineEntry &outline : outlines)
	{
		QString text = outline.text.trimmed();
		if(text.length() > 0)
		{
			text = text.toLower();

			outlineQuery.bindValue(1, text);
			outlineQuery.bindValue(2, outline.destinationPage);
			if(!outlineQuery.exec())
			{
				Logger::error() << "Failed outline insertion " << outlineQuery.lastError() << Qt::endl;
				return false;
			}
			outlineQuery.finish();
			if(!insertOutline(db, fileid, outline.children))
			{
				Logger::error() << "Failed outline insertion (children)) " << outlineQuery.lastError() << Qt::endl;
				return false;
			}
		}
	}
	return true;
}

bool SqliteDbService::runWalCheckpoint()
{
	auto query = QSqlQuery(dbFactory->forCurrentThread());
	return query.exec("PRAGMA wal_checkpoint(TRUNCATE);");
}

QSqlQuery SqliteDbService::exec(QString querystr, std::initializer_list<QVariant> args)
{
	auto query = QSqlQuery(dbFactory->forCurrentThread());
	query.prepare(querystr);
	for(const QVariant &v : args)
	{
		query.addBindValue(v);
	}
	if(!query.exec())
	{
		throw LooqsGeneralException("Error while exec(): " + query.lastError().text() + " for query: " + querystr);
	}
	return query;
}

bool SqliteDbService::execBool(QString querystr, std::initializer_list<QVariant> args)
{
	auto query = exec(querystr, args);
	if(!query.next())
	{
		return false;
	}
	return query.value(0).toBool();
}

SaveFileResult SqliteDbService::saveFile(QFileInfo fileInfo, DocumentProcessResult &processResult, bool pathsOnly)
{
	QString absPath = fileInfo.absoluteFilePath();
	auto mtime = fileInfo.lastModified().toSecsSinceEpoch();
	QChar fileType = fileInfo.isDir() ? 'd' : 'c';
	if(pathsOnly)
	{
		fileType = 'f';
	}

	QSqlDatabase db = dbFactory->forCurrentThread();
	QSqlQuery delQuery(db);
	delQuery.prepare("DELETE FROM file WHERE path = ?");
	delQuery.addBindValue(absPath);

	QSqlQuery inserterQuery(db);
	inserterQuery.prepare("INSERT INTO file(path, mtime, size, filetype) VALUES(?, ?, ?, ?)");
	inserterQuery.addBindValue(absPath);
	inserterQuery.addBindValue(mtime);
	inserterQuery.addBindValue(fileInfo.size());
	inserterQuery.addBindValue(fileType);

	if(!db.transaction())
	{
		Logger::error() << "Failed to open transaction for " << absPath << " : " << db.lastError() << Qt::endl;
		return DBFAIL;
	}

	if(!delQuery.exec())
	{
		Logger::error() << "Failed DELETE query" << delQuery.lastError() << Qt::endl;
		db.rollback();
		return DBFAIL;
	}

	if(!inserterQuery.exec())
	{
		Logger::error() << "Failed INSERT query" << inserterQuery.lastError() << Qt::endl;
		db.rollback();
		return DBFAIL;
	}

	if(!pathsOnly)
	{
		int lastid = inserterQuery.lastInsertId().toInt();
		if(!insertToFTS(false, db, lastid, processResult.pages))
		{
			db.rollback();
			Logger::error() << "Failed to insert data to FTS index " << Qt::endl;
			return DBFAIL;
		}
		if(!insertToFTS(true, db, lastid, processResult.pages))
		{
			db.rollback();
			Logger::error() << "Failed to insert data to FTS index " << Qt::endl;
			return DBFAIL;
		}
		if(!insertOutline(db, lastid, processResult.outlines))
		{
			db.rollback();
			Logger::error() << "Failed to insert outline data " << Qt::endl;
			return DBFAIL;
		}
	}

	if(!db.commit())
	{
		db.rollback();
		Logger::error() << "Failed to commit transaction for " << absPath << " : " << db.lastError() << Qt::endl;
		return DBFAIL;
	}
	return OK;
}

bool SqliteDbService::addTag(QString tag, QString path)
{
	QVector<QString> paths;
	paths.append(path);
	return addTag(tag, paths);
}

bool SqliteDbService::addTag(QString tag, const QVector<QString> &paths)
{
	QSqlDatabase db = dbFactory->forCurrentThread();
	QSqlQuery tagQuery(db);
	QSqlQuery fileTagQuery(db);

	tag = tag.toLower();

	tagQuery.prepare("INSERT OR IGNORE INTO tag (name) VALUES(?)");
	tagQuery.addBindValue(tag);

	fileTagQuery.prepare("INSERT INTO filetag(fileid, tagid) VALUES((SELECT id FROM file WHERE path = ?), (SELECT id "
						 "FROM tag WHERE name = ?))");
	fileTagQuery.bindValue(1, tag);
	if(!db.transaction())
	{
		Logger::error() << "Failed to open transaction to add paths for tag " << tag << " : " << db.lastError()
						<< Qt::endl;
		return false;
	}
	if(!tagQuery.exec())
	{
		db.rollback();
		Logger::error() << "Failed INSERT query" << tagQuery.lastError() << Qt::endl;
		return false;
	}

	for(const QString &path : paths)
	{
		fileTagQuery.bindValue(0, path);
		if(!fileTagQuery.exec())
		{
			db.rollback();
			Logger::error() << "Failed to add paths to tag" << Qt::endl;
			return false;
		}
	}

	if(!db.commit())
	{
		db.rollback();
		Logger::error() << "Failed to commit tag insertion transaction" << db.lastError() << Qt::endl;
		return false;
	}

	return true;
}

bool SqliteDbService::removePathsForTag(QString tag, const QVector<QString> &paths)
{
	QSqlDatabase db = dbFactory->forCurrentThread();
	QSqlQuery tagQuery(db);
	QSqlQuery fileTagQuery(db);

	tag = tag.toLower();

	fileTagQuery.prepare(
		"DELETE FROM filetag WHERE fileid = (SELECT id FROM file WHERE path = ?) AND tagid = (SELECT id "
		"FROM tag WHERE name = ?)");

	fileTagQuery.bindValue(1, tag);
	for(const QString &path : paths)
	{
		fileTagQuery.bindValue(0, path);
		if(!fileTagQuery.exec())
		{
			Logger::error() << "An error occured while trying to remove paths from tag assignment" << Qt::endl;
			return false;
		}
	}
	return true;
}

bool SqliteDbService::deleteTag(QString tag)
{
	QSqlDatabase db = dbFactory->forCurrentThread();
	if(!db.transaction())
	{
		Logger::error() << "Failed to open transaction while trying to delete tag " << tag << " : " << db.lastError()
						<< Qt::endl;
		return false;
	}

	tag = tag.toLower();
	QSqlQuery assignmentDeleteQuery(db);
	assignmentDeleteQuery.prepare("DELETE FROM filetag WHERE tagid = (SELECT id FROM tag WHERE name = ?)");
	assignmentDeleteQuery.addBindValue(tag);
	if(!assignmentDeleteQuery.exec())
	{
		db.rollback();
		Logger::error() << "Error while trying to delete tag: " << db.lastError() << Qt::endl;
		return false;
	}

	QSqlQuery deleteTagQuery(db);
	deleteTagQuery.prepare("DELETE FROM tag WHERE name = ?");
	deleteTagQuery.addBindValue(tag);
	if(!deleteTagQuery.exec())
	{
		db.rollback();
		Logger::error() << "Error while trying to delete tag: " << db.lastError() << Qt::endl;
		return false;
	}

	if(!db.commit())
	{
		db.rollback();
		Logger::error() << "Error while trying to delete tag: " << db.lastError() << Qt::endl;
		return false;
	}
	return true;
}
