#include <quazip.h>
#include <quazipfile.h>
#include "previewgeneratorplaintext.h"
#include "previewgeneratorodt.h"
#include "previewresultodt.h"
#include "../shared/tagstripperprocessor.h"

QSharedPointer<PreviewResult> PreviewGeneratorOdt::generate(RenderConfig config, QString documentPath,
															unsigned int page)
{
	PreviewResultOdt *result = new PreviewResultOdt(documentPath, page);

	QFileInfo info{documentPath};

	QuaZipFile zipFile(documentPath);
	zipFile.setFileName("content.xml");
	if(!zipFile.open(QIODevice::ReadOnly))
	{
		return QSharedPointer<PreviewResult>(result);
	}
	QByteArray entireContent = zipFile.readAll();
	if(entireContent.isEmpty())
	{
		throw LooqsGeneralException("Error while reading content.xml of " + documentPath);
	}
	TagStripperProcessor tsp;
	QString content = tsp.process(entireContent).constFirst().content;

	PreviewGeneratorPlainText plainTextGenerator;
	result->setText(plainTextGenerator.generatePreviewText(content, config, info.fileName()));
	return QSharedPointer<PreviewResult>(result);
}
