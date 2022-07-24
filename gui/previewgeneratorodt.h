#ifndef PREVIEWGENERATORODT_H
#define PREVIEWGENERATORODT_H

#include "previewgenerator.h"
class PreviewGeneratorOdt : public PreviewGenerator
{
  public:
	using PreviewGenerator::PreviewGenerator;

	QSharedPointer<PreviewResult> generate(RenderConfig config, QString documentPath, unsigned int page);
};
#endif // PREVIEWGENERATORODT_H
