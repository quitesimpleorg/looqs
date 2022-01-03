#ifndef PREVIEWGENERATORPDF_H
#define PREVIEWGENERATORPDF_H
#include <poppler-qt5.h>
#include "previewgenerator.h"
#include "previewresultpdf.h"

class PreviewGeneratorPdf : public PreviewGenerator
{
  protected:
	QHash<QString, Poppler::Document *> documentcache;
	Poppler::Document *document(QString path);

  public:
	using PreviewGenerator::PreviewGenerator;

	PreviewResult *generate(RenderConfig config, QString documentPath, unsigned int page);

	~PreviewGeneratorPdf()
	{
		qDeleteAll(documentcache);
	}
};

#endif // PREVIEWGENERATORPDF_H
