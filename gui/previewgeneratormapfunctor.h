#ifndef PREVIEWGENERATORMAPFUNCTOR_H
#define PREVIEWGENERATORMAPFUNCTOR_H

#include "renderconfig.h"
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
	typedef QSharedPointer<PreviewResult> result_type;

	PreviewGeneratorMapFunctor();

	void setRenderConfig(RenderConfig config);

	QSharedPointer<PreviewResult> operator()(const QSharedPointer<PreviewResult> &renderResult);
};

#endif // PREVIEWGENERATORMAPFUNCTOR_H
