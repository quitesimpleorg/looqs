#ifndef PREVIEWGENERATORMAPFUNCTOR_H
#define PREVIEWGENERATORMAPFUNCTOR_H

#include "renderconfig.h"
#include "rendertarget.h"

#include "previewgenerator.h"
class PreviewGeneratorMapFunctor
{

  private:
	enum GeneratorIndex
	{
		PDF = 0,
		LAST_DUMMY
	};
	RenderConfig renderConfig;

  public:
	typedef QByteArray result_type;

	PreviewGeneratorMapFunctor();

	void setRenderConfig(RenderConfig config);

	QByteArray operator()(const RenderTarget &renderTarget);
};

#endif // PREVIEWGENERATORMAPFUNCTOR_H
