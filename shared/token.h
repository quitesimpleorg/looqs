#ifndef TOKEN_H
#define TOKEN_H
#include <QString>

enum TokenType
{
	WORD = 8,
	NEGATION = 16,
	BOOL = 32,
	BOOL_AND,
	BOOL_OR,
	GROUP = 64,
	BRACKET_OPEN,
	BRACKET_CLOSE,
	SORT = 128,
	FILTER_PATH = 256,
	FILTER_PATH_MTIME,
	FILTER_PATH_CONTAINS,
	FILTER_PATH_SIZE,
	FILTER_PATH_ENDS,
	FILTER_PATH_STARTS,
	FILTER_TAG_ASSIGNED,
	FILTER_OUTLINE_CONTAINS,
	FILTER_CONTENT = 512, /* Everything below here is content search (except LIMIT) */
	FILTER_CONTENT_CONTAINS,
	FILTER_CONTENT_PAGE,
	LIMIT = 1024
};

class Token
{
  public:
	Token()
	{
	}
	Token(TokenType type)
	{
		this->type = type;
	}

	Token(TokenType type, QString value)
	{
		this->type = type;
		this->value = value;
	}

	TokenType type;
	QString value;
};

#endif // TOKEN_H
