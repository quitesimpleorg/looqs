#ifndef LOOQSQUERY_H
#define LOOQSQUERY_H
#include <QString>
#include <QVector>
#include "token.h"
/* Fields that can be queried or sorted */
enum QueryField
{
	FILE_PATH,
	FILE_MTIME,
	FILE_SIZE,
	CONTENT_TEXT,
	CONTENT_TEXT_PAGE
};

enum SortOrder
{
	ASC,
	DESC
};

struct SortCondition
{
	QueryField field;
	SortOrder order;
};

enum QueryType
{
	NOTHING = 0,
	PATH_ONLY = FILTER_PATH,
	CONTENT_ONLY = FILTER_CONTENT,
	COMBINED = PATH_ONLY | CONTENT_ONLY
};

class LooqsQuery
{
  private:
	/* Helper field to determine quertype as well as to quickly check what kind of filters etc.
	 * are being used in this query*/
	int tokensMask = 0;
	int limit = -1;
	QVector<Token> tokens;
	QVector<SortCondition> sortConditions;
	void addToken(Token t);
	void updateTokensMask()
	{
		for(const Token &t : qAsConst(tokens))
		{
			this->tokensMask |= t.type;
		}
	}

  public:
	const QVector<Token> &getTokens() const;
	const QVector<SortCondition> &getSortConditions() const;
	QueryType getQueryType();
	int getTokensMask() const
	{
		return tokensMask;
	}
	int getLimit() const
	{
		return limit;
	}
	void setLimit(int limit)
	{
		this->limit = limit;
	}
	bool hasContentSearch() const;
	bool hasOutlineSearch() const;
	bool hasPathSearch() const;

	void addSortCondition(SortCondition sc);

	void setTokens(const QVector<Token> &tokens)
	{
		this->tokens = tokens;
		updateTokensMask();
	}

	static bool checkParanthesis(QString query);
	static LooqsQuery build(QString query, TokenType loneWordsTokenType, bool mergeLoneWords);

	LooqsQuery()
	{
	}

	LooqsQuery(const QVector<Token> &tokens, const QVector<SortCondition> &sortConditions)
	{
		this->tokens = tokens;
		this->sortConditions = sortConditions;
		updateTokensMask();
	}
};

#endif // LOOQSQUERY_H
