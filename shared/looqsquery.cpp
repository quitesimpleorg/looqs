#include <QStack>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QDebug>
#include <optional>
#include <algorithm>
#include "looqsquery.h"

const QVector<Token> &LooqsQuery::getTokens() const
{
	return tokens;
}

const QVector<SortCondition> &LooqsQuery::getSortConditions() const
{
	return sortConditions;
}

QueryType LooqsQuery::getQueryType()
{
	return static_cast<QueryType>(tokensMask & COMBINED);
}

bool LooqsQuery::hasContentSearch() const
{
	return (this->getTokensMask() & FILTER_CONTENT) == FILTER_CONTENT;
}

bool LooqsQuery::hasPathSearch() const
{
	return (this->getTokensMask() & FILTER_PATH) == FILTER_PATH;
}

void LooqsQuery::addSortCondition(SortCondition sc)
{
	this->sortConditions.append(sc);
}

bool LooqsQuery::checkParanthesis(QString expression)
{
	QStack<QChar> open;
	QStack<QChar> close;

	bool inQuotes = false;

	for(QChar &c : expression)
	{
		if(!inQuotes)
		{
			if(c == '(')
			{
				open.push(c);
			}
			if(c == ')')
			{
				close.push(c);
			}
		}
		if(c == '"')
		{
			inQuotes = !inQuotes;
		}
	}
	if(open.size() != close.size())
	{
		return false;
	}
	while(!open.empty() && !close.empty())
	{
		QChar o = open.pop();
		QChar c = close.pop();
		if(o != '(' && c != ')')
		{
			return false;
		}
	}
	return true;
}

std::optional<QueryField> fromString(QString fieldString)
{
	if(fieldString == "path" || fieldString == "file.path")
	{
		return FILE_PATH;
	}
	else if(fieldString == "mtime" || fieldString == "file.mtime")
	{
		return FILE_MTIME;
	}
	else if(fieldString == "size" || fieldString == "file.size")
	{
		return FILE_SIZE;
	}
	else if(fieldString == "content.text")
	{
		return CONTENT_TEXT;
	}
	else if(fieldString == "content.page" || fieldString == "page")
	{
		return CONTENT_TEXT_PAGE;
	}
	return {};
}

// sort:(mtime desc, page asc)
QVector<SortCondition> createSortConditions(QString sortExpression)
{
	QVector<SortCondition> result;
	QStringList splitted_inner = sortExpression.split(",");
	for(int i = 0; i < splitted_inner.length(); i++)
	{
		QStringList splitted = splitted_inner[i].split(" ");
		if(splitted.length() < 1 || splitted.length() > 2)
		{
			throw LooqsGeneralException("sort specifier must have format [field] (asc|desc)");
		}

		QString field = splitted[0];
		auto queryField = fromString(field);
		if(!queryField)
		{
			throw LooqsGeneralException("Unknown sort field supplied");
		}

		SortOrder order;
		if(splitted.length() == 2)
		{
			QString orderstr = splitted[1];
			if(orderstr.compare("asc", Qt::CaseInsensitive) == 0)
			{
				order = ASC;
			}
			else if(orderstr.compare("desc", Qt::CaseInsensitive) == 0)
			{
				order = DESC;
			}
			else
			{
				throw LooqsGeneralException("Unknown order specifier: " + orderstr);
			}
		}
		else
		{
			order = ASC;
		}

		SortCondition condition;
		condition.field = queryField.value();
		condition.order = order;
		result.append(condition);
	}

	return result;
}

void LooqsQuery::addToken(Token t)
{
	tokens.append(t);
	tokensMask |= t.type;
}

/* Builds the query from the supplied expression
 *
 * AND is the default boolean operator, when the user does not provide any
 * thus, "Downloads zip" becomes essentailly "path.contains:(Downloads) AND path.contains:(zip)"
 *
 * TODO: It's a bit ugly still*/
LooqsQuery LooqsQuery::build(QString expression, TokenType loneWordsTokenType, bool mergeLoneWords)
{
	if(expression.isEmpty())
	{
		return LooqsQuery{};
	}
	if(!checkParanthesis(expression))
	{
		throw LooqsGeneralException("Invalid paranthesis");
	}

	QStringList loneWords;
	LooqsQuery result;
	QRegularExpression rx(
		"((?<filtername>(\\.|\\w)+):(?<args>\\((?<innerargs>[^\\)]+)\\)|([\\p{L},])+)|(?<boolean>AND|OR)"
		"|(?<negation>!)|(?<bracket>\\(|\\))|(?<loneword>[\"\\p{L}]+))");
	QRegularExpressionMatchIterator i = rx.globalMatch(expression);
	auto previousWasBool = [&result] { return !result.tokens.empty() && ((result.tokens.last().type & BOOL) == BOOL); };
	auto previousWas = [&result](TokenType t) { return !result.tokens.empty() && (result.tokens.last().type == t); };

	while(i.hasNext())
	{
		QRegularExpressionMatch m = i.next();
		QString boolean = m.captured("boolean");
		QString negation = m.captured("negation");
		QString filtername = m.captured("filtername");
		QString bracket = m.captured("bracket");
		QString loneword = m.captured("loneword");

		if(boolean != "")
		{
			if(previousWasBool())
			{
				throw LooqsGeneralException("Can't have two booleans following each other");
			}
			if(previousWas(NEGATION))
			{
				throw LooqsGeneralException("Can't have a negation preceeding a boolean");
			}
			if(boolean == "AND")
			{
				result.addToken(Token(BOOL_AND));
			}
			else if(boolean == "OR")
			{
				result.addToken(Token(BOOL_OR));
			}
		}
		if(negation != "")
		{
			if(previousWas(NEGATION))
			{
				throw LooqsGeneralException("Can't have two negations following each other");
			}
			result.addToken(Token(NEGATION));
		}
		if(bracket != "")
		{
			if(bracket == "(")
			{
				result.addToken(Token(BRACKET_OPEN));
			}
			else
			{
				result.addToken(Token(BRACKET_CLOSE));
			}
		}

		if(loneword != "")
		{
			if(mergeLoneWords)
			{
				loneWords.append(loneword);
			}
			else
			{
				result.addToken(Token(loneWordsTokenType, loneword));
			}
		}

		if(filtername != "")
		{
			TokenType tokenType = WORD;
			QString value = m.captured("innerargs");
			if(value == "")
			{
				value = m.captured("args");
			}
			if(value == "")
			{
				throw LooqsGeneralException("value cannot be empty for filters");
			}

			if(filtername == "p" || filtername == "path.contains")
			{
				tokenType = FILTER_PATH_CONTAINS;
			}
			else if(filtername == "pb" || filtername == "path.starts")
			{
				tokenType = FILTER_PATH_STARTS;
			}
			else if(filtername == "pe" || filtername == "path.ends")
			{
				tokenType = FILTER_PATH_ENDS;
			}
			else if(filtername == "file.size" || filtername == "size")
			{
				tokenType = FILTER_PATH_SIZE;
			}
			else if(filtername == "c" || filtername == "contains")
			{
				tokenType = FILTER_CONTENT_CONTAINS;
			}
			else if(filtername == "page" || filtername == "content.page")
			{
				tokenType = FILTER_CONTENT_PAGE;
			}
			// TODO: given this is not really a "filter", this feels slightly misplaced here
			else if(filtername == "sort")
			{
				if(!result.sortConditions.empty())
				{
					throw LooqsGeneralException("Two sort statements are illegal");
				}
				result.sortConditions = createSortConditions(value);
				continue;
			}
			else if(filtername == "limit")
			{
				result.limit = value.toInt();
				continue;
			}
			else
			{
				throw LooqsGeneralException("Unknown filter provided!");
			}
			result.addToken(Token(tokenType, value));
		}
	}

	if(mergeLoneWords)
	{
		QString mergedLoneWords = loneWords.join(' ');
		if(!mergedLoneWords.isEmpty())
		{
			result.addToken(Token(loneWordsTokenType, mergedLoneWords));
		}
	}

	/* Add our default implicit AND boolean condition where appropriate */
	QVector<Token> newTokens;

	TokenType prevType = BOOL_AND;
	int needsBoolean = FILTER_CONTENT | FILTER_PATH | NEGATION;
	for(Token &t : result.tokens)
	{
		if(t.type == BRACKET_OPEN || t.type & needsBoolean)
		{
			if(!((prevType & BOOL) == BOOL) && !((prevType & NEGATION) == NEGATION) &&
			   !((prevType & BRACKET_OPEN) == BRACKET_OPEN))
			{
				newTokens.append(Token(BOOL_AND));
			}
		}
		prevType = t.type;
		newTokens.append(t);
	}
	result.tokens = newTokens;

	bool contentsearch = result.hasContentSearch();
	bool sortsForContent = std::any_of(result.sortConditions.begin(), result.sortConditions.end(),
									   [](SortCondition c) { return c.field == CONTENT_TEXT; });

	if(!contentsearch && sortsForContent)
	{
		throw LooqsGeneralException("We cannot sort by text if we don't search for it");
	}

	return result;
}
