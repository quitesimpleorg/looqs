#include <QFile>
#include <QDataStream>
#include <QTextCodec>
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
		QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8());
		if(codec != nullptr)
		{
			return codec->toUnicode(data);
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
