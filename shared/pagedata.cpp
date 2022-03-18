#include "pagedata.h"

QDataStream &operator<<(QDataStream &out, const PageData &pd)
{
	out << pd.pagenumber << pd.content;
	return out;
}

QDataStream &operator>>(QDataStream &in, PageData &pd)
{
	in >> pd.pagenumber >> pd.content;
	return in;
}
