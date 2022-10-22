#include "../shared/common.h"
#include "previewgenerator.h"
#include <QMutexLocker>
#include "previewgeneratorpdf.h"
#include "previewgeneratorplaintext.h"
#include "previewgeneratorodt.h"

static PreviewGenerator *plainTextGenerator = new PreviewGeneratorPlainText();

static QHash<QString, PreviewGenerator *> generators{
	{"pdf", new PreviewGeneratorPdf()}, {"txt", plainTextGenerator},  {"md", plainTextGenerator},
	{"py", plainTextGenerator},			{"java", plainTextGenerator}, {"js", plainTextGenerator},
	{"cpp", plainTextGenerator},		{"c", plainTextGenerator},	  {"sql", plainTextGenerator},
	{"odt", new PreviewGeneratorOdt()}};

static QMutex generatorsMutex;
PreviewGenerator *PreviewGenerator::get(QFileInfo &info)
{
	QMutexLocker locker(&generatorsMutex);
	PreviewGenerator *result = generators.value(info.suffix(), nullptr);
	locker.unlock();
	if(result == nullptr)
	{
		if(Common::isTextFile(info))
		{
			return plainTextGenerator;
		}
	}
	return result;
}
