#ifndef RENDERCONFIG_H
#define RENDERCONFIG_H
#include <QVector>

struct RenderConfig
{
	double scaleX = 50 / 100.;
	double scaleY = scaleX;
	QVector<QString> wordsToHighlight;
};

#endif // RENDERCONFIG_H
