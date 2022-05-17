#include <QDataStream>

#include "rendertarget.h"
QDataStream &operator<<(QDataStream &out, const RenderTarget &rc)
{
	out << rc.path << rc.page;
	return out;
}

QDataStream &operator>>(QDataStream &in, RenderTarget &rc)
{
	in >> rc.path >> rc.page;
	return in;
}
