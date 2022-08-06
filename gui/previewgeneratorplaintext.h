#ifndef PREVIEWGENERATORPLAINTEXT_H
#define PREVIEWGENERATORPLAINTEXT_H
#include "previewgenerator.h"

class PreviewGeneratorPlainText : public PreviewGenerator
{
  protected:
	const unsigned int MAX_SNIPPETS = 7;

  public:
	using PreviewGenerator::PreviewGenerator;
	QString generatePreviewText(QString content, RenderConfig config, QString fileName);
	QSharedPointer<PreviewResult> generate(RenderConfig config, QString documentPath, unsigned int page);
};

#endif // PREVIEWGENERATORPLAINTEXT_H
