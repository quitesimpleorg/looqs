#include "previewgenerator.h"
#include "previewgeneratorpdf.h"
#include "previewgeneratorplaintext.h"

static PreviewGenerator *plainTextGenerator = new PreviewGeneratorPlainText();

static QMap<QString, PreviewGenerator *> generators{
	{"pdf", new PreviewGeneratorPdf()}, {"txt", plainTextGenerator},  {"md", plainTextGenerator},
	{"py", plainTextGenerator},			{"java", plainTextGenerator}, {"js", plainTextGenerator},
	{"cpp", plainTextGenerator},		{"c", plainTextGenerator},	  {"sql", plainTextGenerator}};

PreviewGenerator *PreviewGenerator::get(QFileInfo &info)
{
	return generators.value(info.suffix(), nullptr);
}
