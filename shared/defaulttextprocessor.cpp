#include <QFile>
#include <QDataStream>
#include <QStringDecoder>
#include <QDebug>
#include "defaulttextprocessor.h"

DefaultTextProcessor::DefaultTextProcessor()
{
}

QString DefaultTextProcessor::processText(const QByteArray &data) const
{
	QString encoding = encodingDetector.detectEncoding(data);
	if(!encoding.isEmpty())
	{
		QStringDecoder decoder = QStringDecoder(encoding.toStdString().c_str());
		if(decoder.isValid())
		{
			return decoder(data);
		}
		qWarning() << "No codec found for " << encoding;
		return QString(data);
	}
	return {};
}

DocumentProcessResult DefaultTextProcessor::process(const QByteArray &data) const
{
	DocumentProcessResult result;
	result.pages.append({0, processText(data)});
	return result;
}
