#ifndef RENDERCONFIG_H
#define RENDERCONFIG_H
#include <QVector>
#include <QDataStream>

struct RenderConfig
{
	double scaleX = 50 / 100.;
	double scaleY = scaleX;
	QVector<QString> wordsToHighlight;
	friend QDataStream &operator<<(QDataStream &out, const RenderConfig &rc);
	friend QDataStream &operator>>(QDataStream &in, RenderConfig &rc);
};

QDataStream &operator<<(QDataStream &out, const RenderConfig &rc);
QDataStream &operator>>(QDataStream &in, RenderConfig &rc);
#endif // RENDERCONFIG_H
