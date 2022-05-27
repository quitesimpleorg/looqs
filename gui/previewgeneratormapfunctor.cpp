#include "previewgeneratormapfunctor.h"
#include "previewgeneratorpdf.h"

PreviewGeneratorMapFunctor::PreviewGeneratorMapFunctor()
{
}

void PreviewGeneratorMapFunctor::setRenderConfig(RenderConfig config)
{
	this->renderConfig = config;
}

QByteArray PreviewGeneratorMapFunctor::operator()(const RenderTarget &renderTarget)
{
	QFileInfo info{renderTarget.path};
	PreviewGenerator *previewGenerator = PreviewGenerator::get(info);
	if(previewGenerator == nullptr)
	{
		return QByteArray{};
	}
	auto preview = previewGenerator->generate(this->renderConfig, renderTarget.path, renderTarget.page);

	return preview->serialize();
}
