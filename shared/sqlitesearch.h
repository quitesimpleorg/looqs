#ifndef SQLITESEARCH_H
#define SQLITESEARCH_H
#include <QSqlDatabase>
#include <QPair>
#include "searchresult.h"
#include "token.h"
#include "../shared/looqsquery.h"
class SqliteSearch
{

  public:
	SqliteSearch(QSqlDatabase &db);
	QVector<SearchResult> search(const LooqsQuery &query);

  private:
	QSqlDatabase *db;
	QSqlQuery makeSqlQuery(const LooqsQuery &query);
	QString fieldToColumn(QueryField field);
	QPair<QString, QVector<QString>> createSql(const Token &token);
	QString createSortSql(const QVector<SortCondition> sortConditions);
	QString escapeFtsArgument(QString ftsArg);
};

#endif // SQLITESEARCH_H
