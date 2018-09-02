#include "searchworker.h"

#include <QRegularExpression>
#include <QDebug>
#include <QSqlError>
SearchWorker::SearchWorker()
{
}
SearchWorker::SearchWorker(const QString &dbpath)
{
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dbpath);
	if(!db.open())
	{
		qDebug() << "failed to open database";
	}
	queryContent = new QSqlQuery(db);
	queryFile = new QSqlQuery(db);
	queryFile->prepare("SELECT path, mtime FROM file WHERE path LIKE ? ORDER BY mtime DESC");

	queryContent->prepare("SELECT file.path, content.page, file.mtime FROM file INNER JOIN content ON file.id = "
						  "content.fileid INNER JOIN content_fts ON content.id = content_fts.ROWID WHERE "
						  "content_fts.content MATCH ? ORDER By file.mtime DESC, content.page ASC");
}

QVector<SearchWorker::Command> SearchWorker::tokenize(QString expression)
{
	// TODO: merge lonewords
	QVector<Command> result;
	QRegularExpression rx("((?<filtername>(\\.|\\w)+):(?<args>\\((?<innerargs>[^\\)]+)\\)|(\\w)+)|(?<boolean>AND|OR|!)|"
						  "(?<bracket>\\(|\\))|(?<loneword>\\w+))");
	QRegularExpressionMatchIterator i = rx.globalMatch(expression);
	bool wasbool = true;
	while(i.hasNext())
	{
		QRegularExpressionMatch m = i.next();
		QString boolean = m.captured("boolean");
		QString filtername = m.captured("filtername");
		QString bracket = m.captured("bracket");
		QString loneword = m.captured("loneword");
		if(boolean != "")
		{
			/* if(wasbool)
			 {
				 throw new std::runtime_error("Bool after Bool is invalid");
			 }*/
			wasbool = true;
			result.append(Command(boolean));
		}

		if(bracket != "")
		{
			if(!wasbool)
			{
				if(bracket == "(")
				{
					result.append(Command("AND"));
				}
			}
			result.append(Command(bracket));
		}

		if(loneword != "")
		{
			if(!wasbool)
			{
				result.append(Command("AND"));
			}
			wasbool = false;
			result.append(Command("contains", loneword));
		}
		if(filtername != "")
		{
			if(!wasbool)
			{
				result.append(Command("AND"));
			}
			wasbool = false;
			QString value = m.captured("innerargs");
			if(value == "")
				value = m.captured("args");
			result.append(Command(filtername, value));
		}
	}
	return result;
}

QString SearchWorker::createSql(const SearchWorker::Command &cmd)
{
	QString key = cmd.key;
	QString value = cmd.value;
	if(key == "AND" || key == "OR" || key == "(" || key == ")")
	{
		return " " + key + " ";
	}
	if(key == "!")
	{
		return " NOT ";
	}
	if(key == "path.starts")
	{
		return " file.path LIKE '" + value + "%' ";
	}
	if(key == "path.ends")
	{
		return " file.path LIKE '%" + value + "' ";
	}
	if(key == "path.contains" || key == "inpath")
	{
		return " file.path LIKE '%" + value + "%' ";
	}
	if(key == "page")
	{
		return " content.page = " + value;
	}
	if(key == "contains")
	{
		return " ( COALESCE( (SELECT 1 FROM content_fts WHERE content_fts.content MATCH '" + value +
			   "' AND content_fts.ROWID= content.id), 0 ) )";
	}
	qDebug() << "NOHIT" << key;
	// TODO: exception?
	return "NOTHING";
}

QString SearchWorker::makeSql(const QVector<SearchWorker::Command> &tokens)
{
	QString result;
	for(const Command &c : tokens)
	{
		result += createSql(c);
	}
	return result;
}
void SearchWorker::searchForFile(const QString &query)
{
	QVector<SearchResult> results;
	queryFile->addBindValue("%" + query + "%");
	queryFile->exec();
	while(queryFile->next())
	{
		SearchResult result;
		result.page = 0;
		result.path = queryFile->value(0).toString();
		result.mtime = queryFile->value(1).toUInt();

		results.append(result);
	}
	emit searchResultsReady(results);
}
void SearchWorker::searchForContent(const QString &query)
{
	QVector<SearchResult> results;
	queryContent->addBindValue(query);
	queryContent->exec();
	while(queryContent->next())
	{
		SearchResult result;

		result.path = queryContent->value(0).toString();
		result.page = queryContent->value(1).toUInt();
		result.mtime = queryContent->value(2).toUInt();
		results.append(result);
	}
	emit searchResultsReady(results);
}

void SearchWorker::search(const QString &query)
{
	QSqlQuery dbquery(db);
	QVector<SearchResult> results;
	QString whereSql = makeSql(tokenize(query));
	QString prep =
		"SELECT file.path AS path, content.page AS page, file.mtime AS mtime FROM file INNER JOIN content ON file.id = "
		"content.fileid INNER JOIN content_fts ON content.id = content_fts.ROWID WHERE 1=1 AND " +
		whereSql + " ORDER By file.mtime DESC, content.page ASC";
	dbquery.prepare(prep);
	dbquery.exec();
	qDebug() << "prepped: " << prep;
	qDebug() << dbquery.lastError();
	while(dbquery.next())
	{
		SearchResult result;

		result.path = dbquery.value("path").toString();
		result.page = dbquery.value("page").toUInt();
		result.mtime = dbquery.value("mtime").toUInt();
		results.append(result);
	}
	emit searchResultsReady(results);
}
