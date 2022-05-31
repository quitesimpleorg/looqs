#ifndef WILDCARDMATCHER_H
#define WILDCARDMATCHER_H
#include <QStringList>
#include <QRegExp>
class WildcardMatcher
{
  private:
	QVector<QRegExp> regexes;
	QStringList patterns;

  public:
	WildcardMatcher();
	bool match(QString haystack) const;
	void setPatterns(QStringList patterns);
};

#endif // WILDCARDMATCHER_H
