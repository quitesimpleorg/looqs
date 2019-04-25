
#include <QRegularExpression>
#include <QDebug>
#include <QSqlError>
#include <QStack>
#include "searchworker.h"
#include "../shared/sqlitesearch.h"
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
}

void SearchWorker::search(const QString &query)
{
	SqliteSearch searcher(db);
	emit searchResultsReady(searcher.search(query));
}
