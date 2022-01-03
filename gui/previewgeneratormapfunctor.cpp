#include "previewgeneratormapfunctor.h"
#include "previewgeneratorpdf.h"

PreviewGeneratorMapFunctor::PreviewGeneratorMapFunctor()
{
	generator[GeneratorIndex::PDF] = new PreviewGeneratorPdf();
}

PreviewGenerator *PreviewGeneratorMapFunctor::getGenerator(QString filePath)
{
	/* Dirty, but that's all we have at this point */
	if(filePath.endsWith(".pdf"))
	{
		return generator[GeneratorIndex::PDF];
	}
	return nullptr;
}

PreviewGeneratorMapFunctor::~PreviewGeneratorMapFunctor()
{
	for(int i = GeneratorIndex::PDF; i < GeneratorIndex::LAST_DUMMY; i++)
	{
		// delete generator[i];
		generator[i] = nullptr;
	}
}

void PreviewGeneratorMapFunctor::setRenderConfig(RenderConfig config)
{
	this->renderConfig = config;
}

QSharedPointer<PreviewResult> PreviewGeneratorMapFunctor::operator()(const QSharedPointer<PreviewResult> &renderResult)
{
	PreviewGenerator *previewGenerator = getGenerator(renderResult->getDocumentPath());
	if(previewGenerator == nullptr)
	{
		return QSharedPointer<PreviewResult>();
	}
	auto preview =
		previewGenerator->generate(this->renderConfig, renderResult->getDocumentPath(), renderResult->getPage());

	return QSharedPointer<PreviewResult>(preview);
}
