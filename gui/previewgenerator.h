#ifndef PREVIEWGENERATOR_H
#define PREVIEWGENERATOR_H
#include <QVector>
#include <QSharedPointer>
#include <QFileInfo>
#include "previewresult.h"
#include "renderconfig.h"

class PreviewGenerator
{
  public:
	virtual PreviewResult *generate(RenderConfig config, QString documentPath, unsigned int page) = 0;
	virtual ~PreviewGenerator()
	{
	}

	static PreviewGenerator *get(QFileInfo &info);
};

#endif // PREVIEWGENERATOR_H
