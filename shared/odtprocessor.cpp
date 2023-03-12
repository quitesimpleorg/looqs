#include <quazip.h>
#include <quazipfile.h>
#include "odtprocessor.h"
#include "tagstripperprocessor.h"

QVector<PageData> OdtProcessor::process(const QByteArray & /*data*/) const
{
	throw LooqsGeneralException("Not implemented yet");
}

QVector<PageData> OdtProcessor::process(QString path) const
{
	QuaZipFile zipFile(path);
	zipFile.setFileName("content.xml");
	if(!zipFile.open(QIODevice::ReadOnly))
	{
		throw LooqsGeneralException("Error while opening file " + path);
	}
	QByteArray entireContent = zipFile.readAll();
	if(entireContent.isEmpty())
	{
		throw LooqsGeneralException("Error while reading content.xml of " + path);
	}
	TagStripperProcessor tsp;
	return tsp.process(entireContent);
}
