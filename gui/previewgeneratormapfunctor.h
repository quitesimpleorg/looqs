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
	PreviewGenerator *generator[LAST_DUMMY];
	PreviewGenerator *getGenerator(QString filePath);

  public:
	typedef QSharedPointer<PreviewResult> result_type;

	PreviewGeneratorMapFunctor();

	~PreviewGeneratorMapFunctor();

	void setRenderConfig(RenderConfig config);

	QSharedPointer<PreviewResult> operator()(const QSharedPointer<PreviewResult> &renderResult);
};

#endif // PREVIEWGENERATORMAPFUNCTOR_H
