#include "renderconfig.h"

QDataStream &operator<<(QDataStream &out, const RenderConfig &rc)
{
	out << rc.scaleX;
	out << rc.scaleY;
	out << rc.wordsToHighlight;
	return out;
}

QDataStream &operator>>(QDataStream &in, RenderConfig &rc)
{
	in >> rc.scaleX;
	in >> rc.scaleY;
	in >> rc.wordsToHighlight;
	return in;
}
