#include "../shared/common.h"
#include "previewgenerator.h"
#include "previewgeneratorpdf.h"
#include "previewgeneratorplaintext.h"
#include "previewgeneratorodt.h"

static PreviewGenerator *plainTextGenerator = new PreviewGeneratorPlainText();

static QMap<QString, PreviewGenerator *> generators{
	{"pdf", new PreviewGeneratorPdf()}, {"txt", plainTextGenerator},  {"md", plainTextGenerator},
	{"py", plainTextGenerator},			{"java", plainTextGenerator}, {"js", plainTextGenerator},
	{"cpp", plainTextGenerator},		{"c", plainTextGenerator},	  {"sql", plainTextGenerator},
	{"odt", new PreviewGeneratorOdt()}};

PreviewGenerator *PreviewGenerator::get(QFileInfo &info)
{
	PreviewGenerator *result = generators.value(info.suffix(), nullptr);
	if(result == nullptr)
	{
		if(Common::isTextFile(info))
		{
			return plainTextGenerator;
		}
	}
	return result;
}
