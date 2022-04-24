#include "previewgeneratormapfunctor.h"
#include "previewgeneratorpdf.h"

PreviewGeneratorMapFunctor::PreviewGeneratorMapFunctor()
{
}

void PreviewGeneratorMapFunctor::setRenderConfig(RenderConfig config)
{
	this->renderConfig = config;
}

QSharedPointer<PreviewResult> PreviewGeneratorMapFunctor::operator()(const QSharedPointer<PreviewResult> &renderResult)
{
	QFileInfo info{renderResult->getDocumentPath()};
	PreviewGenerator *previewGenerator = PreviewGenerator::get(info);
	if(previewGenerator == nullptr)
	{
		return QSharedPointer<PreviewResult>();
	}
	auto preview =
		previewGenerator->generate(this->renderConfig, renderResult->getDocumentPath(), renderResult->getPage());

	return QSharedPointer<PreviewResult>(preview);
}
