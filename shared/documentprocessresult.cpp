#include "documentprocessresult.h"
#include <QDebug>
QDataStream &operator<<(QDataStream &out, const DocumentProcessResult &pd)
{
	out << pd.pages.size();
	out << pd.outlines.size();
	for(const PageData &pd : pd.pages)
	{
		out << pd;
	}
	for(const DocumentOutlineEntry &outline : pd.outlines)
	{
		out << outline;
	}
	return out;
}

QDataStream &operator>>(QDataStream &in, DocumentProcessResult &pd)
{
	qsizetype numPages, numOutlines;
	in >> numPages;
	in >> numOutlines;

	for(int i = 0; i < numPages; i++)
	{
		PageData data;
		in >> data;
		pd.pages.append(data);
	}

	for(int i = 0; i < numOutlines; i++)
	{
		DocumentOutlineEntry outline;
		in >> outline;
		pd.outlines.append(outline);
	}

	return in;
}
