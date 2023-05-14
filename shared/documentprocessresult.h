#ifndef DOCUMENTPROCESSRESULT_H
#define DOCUMENTPROCESSRESULT_H
#include <pagedata.h>
#include <documentoutlineentry.h>

class DocumentProcessResult
{
  public:
	QVector<PageData> pages;
	QVector<DocumentOutlineEntry> outlines;
};
Q_DECLARE_METATYPE(DocumentProcessResult);

QDataStream &operator<<(QDataStream &out, const DocumentProcessResult &pd);
QDataStream &operator>>(QDataStream &in, DocumentProcessResult &pd);

#endif // DOCUMENTPROCESSRESULT_H
