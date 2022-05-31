#include "wildcardmatcher.h"

void WildcardMatcher::setPatterns(QStringList patterns)
{
	this->regexes.clear();
	for(QString &str : patterns)
	{
		QRegExp regexp;
		regexp.setPattern(str);
		regexp.setPatternSyntax(QRegExp::WildcardUnix);
		this->regexes.append(regexp);
	}
}

WildcardMatcher::WildcardMatcher()
{
}

bool WildcardMatcher::match(QString haystack) const
{
	for(const QRegExp &regexp : this->regexes)
	{
		if(regexp.exactMatch(haystack))
		{
			return true;
		}
	}
	return false;
}
