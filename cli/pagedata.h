#ifndef PAGEDATA_H
#define PAGEDATA_H
#include <QString>
#include <QMetaType>
#include <QDataStream>

class PageData
{
  public:
	unsigned int pagenumber = 0;
	QString content;

	PageData()
	{
	}

	PageData(unsigned int pagenumber, QString content)
	{
		this->pagenumber = pagenumber;
		this->content = content;
	}
};

Q_DECLARE_METATYPE(PageData);

QDataStream &operator<<(QDataStream &out, const PageData &pd);
QDataStream &operator>>(QDataStream &in, PageData &pd);

#endif // PAGEDATA_H
