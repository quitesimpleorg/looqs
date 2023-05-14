#include "documentoutlineentry.h"

DocumentOutlineEntry::DocumentOutlineEntry()
{
}

QDataStream &operator<<(QDataStream &out, const DocumentOutlineEntry &pd)
{
	out << pd.text << pd.type << pd.destinationPage;
	out << pd.children.size();
	for(const DocumentOutlineEntry &entry : pd.children)
	{
		out << entry;
	}
	return out;
}

QDataStream &operator>>(QDataStream &in, DocumentOutlineEntry &pd)
{
	in >> pd.text >> pd.type >> pd.destinationPage;

	int numChildren;
	in >> numChildren;
	for(int i = 0; i < numChildren; i++)
	{
		DocumentOutlineEntry entry;
		in >> entry;
		pd.children.append(entry);
	}
	return in;
}
