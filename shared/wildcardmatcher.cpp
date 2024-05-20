#include "wildcardmatcher.h"

void WildcardMatcher::setPatterns(QStringList patterns)
{
	this->regexes.clear();
	for(QString &str : patterns)
	{
		QRegularExpression regexp;
		regexp.setPattern(QRegularExpression::wildcardToRegularExpression(str));
		this->regexes.append(regexp);
	}
}

WildcardMatcher::WildcardMatcher()
{
}

bool WildcardMatcher::match(QString haystack) const
{
	for(const QRegularExpression &regexp : this->regexes)
	{
		QRegularExpressionMatch match = regexp.match(haystack);
		if(match.hasMatch())
		{
			return true;
		}
	}
	return false;
}
