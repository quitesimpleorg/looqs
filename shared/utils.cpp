#include <QDebug>
#include "utils.h"
Utils::Utils()
{
}

QByteArray Utils::readFile(QString path)
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly))
	{
		throw LooqsGeneralException("Failed to open file: " + path);
	}
	QByteArray data = file.readAll();
	if(data.isEmpty() && file.error() != QFileDevice::FileError::NoError)
	{
		throw LooqsGeneralException("Error reading file: " + path + ", Error: " + QString::number(file.error()));
	}
	return data;
}
