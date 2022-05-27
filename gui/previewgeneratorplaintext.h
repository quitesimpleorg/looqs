#ifndef PREVIEWGENERATORPLAINTEXT_H
#define PREVIEWGENERATORPLAINTEXT_H
#include "previewgenerator.h"

class PreviewGeneratorPlainText : public PreviewGenerator
{
  public:
	using PreviewGenerator::PreviewGenerator;
	QSharedPointer<PreviewResult> generate(RenderConfig config, QString documentPath, unsigned int page);
};

#endif // PREVIEWGENERATORPLAINTEXT_H
