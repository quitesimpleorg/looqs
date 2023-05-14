#ifndef DOCUMENTOUTLINEENTRY_H
#define DOCUMENTOUTLINEENTRY_H
#include <QMetaType>
#include <QDataStream>
#include <QString>

enum OutlineDestinationType
{
	OUTLINE_DESTINATION_TYPE_NONE,
	OUTLINE_DESTINATION_TYPE_PAGE
	/* In the future, links, or #anchors are possible */
};

class DocumentOutlineEntry
{
  public:
	DocumentOutlineEntry();
	QVector<DocumentOutlineEntry> children;
	OutlineDestinationType type;
	QString text;
	unsigned int destinationPage;
};

Q_DECLARE_METATYPE(DocumentOutlineEntry);

QDataStream &operator<<(QDataStream &out, const DocumentOutlineEntry &pd);
QDataStream &operator>>(QDataStream &in, DocumentOutlineEntry &pd);

#endif // DOCUMENTOUTLINEENTRY_H
