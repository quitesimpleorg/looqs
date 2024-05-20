#ifndef WILDCARDMATCHER_H
#define WILDCARDMATCHER_H
#include <QStringList>
#include <QRegularExpression>
class WildcardMatcher
{
  private:
	QVector<QRegularExpression> regexes;
	QStringList patterns;

  public:
	WildcardMatcher();
	bool match(QString haystack) const;
	void setPatterns(QStringList patterns);
};

#endif // WILDCARDMATCHER_H
