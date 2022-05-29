#include <QLocalSocket>
#include <QApplication>
#include "ipc.h"
#include "ipcpreviewclient.h"
#include "previewresultpdf.h"
#include "previewresultplaintext.h"

bool IPCPreviewClient::connect()
{
	if(socket->state() == QLocalSocket::ConnectedState)
	{
		socket->disconnectFromServer();
		if(socket->state() == QLocalSocket::ConnectedState)
		{
			socket->waitForDisconnected(100);
		}
	}
	socket->connectToServer(socketPath);
	socket->waitForConnected(100);
	return socket->state() == QLocalSocket::ConnectedState;
}

QSharedPointer<PreviewResult> IPCPreviewClient::deserialize(QByteArray &array)
{
	QDataStream stream{&array, QIODevice::ReadOnly};

	PreviewResultType type;
	stream >> type;
	if(type == PreviewResultType::PDF)
	{
		return PreviewResultPdf::deserialize(array);
	}
	if(type == PreviewResultType::PlainText)
	{
		return PreviewResultPlainText::deserialize(array);
	}
	return QSharedPointer<PreviewResult>(nullptr);
}

IPCPreviewClient::IPCPreviewClient()
{
	this->socket = new QLocalSocket(this);
}

void IPCPreviewClient::setSocketPath(QString socketPath)
{
	this->socketPath = socketPath;
}

void IPCPreviewClient::startGeneration(RenderConfig config, const QVector<RenderTarget> &targets)
{
	this->start(config, targets);
}

void IPCPreviewClient::start(RenderConfig config, const QVector<RenderTarget> &targets)
{
	if(targets.count() == 0)
	{
		return;
	}

	if(!connect() || !socket->isOpen())
	{
		emit error("Could not connect to IPC worker");
		return;
	}

	if(socket->isOpen() && socket->isWritable())
	{
		QDataStream stream(socket);
		stream << GeneratePreviews;
		stream << config;
		stream << targets;
		socket->flush();

		int numTarget = 0;
		if(socket->isOpen() && socket->isReadable() && socket->state() == QLocalSocket::ConnectedState)
		{
			do
			{
				socket->waitForReadyRead(100);
				stream.startTransaction();
				stream >> numTarget;
			} while(!stream.commitTransaction() && socket->state() == QLocalSocket::ConnectedState);
			if(numTarget != targets.count())
			{
				emit error("IPC Error: Server reports less targets than it should");
				return;
			}
		}
		else
		{
			emit error("Error while trying to process previews: " + socket->errorString());
			return;
		}

		int processed = 0;
		++this->currentPreviewGeneration;
		while(socket->isOpen() && socket->isReadable() && processed < targets.count())
		{
			QByteArray array;
			do
			{
				socket->waitForReadyRead(100);
				stream.startTransaction();
				stream >> array;
			} while(!stream.commitTransaction() && socket->state() == QLocalSocket::ConnectedState);
			emit previewReceived(deserialize(array), this->currentPreviewGeneration);
			++processed;
		}
		if(processed != targets.count())
		{
			emit error("IPC worker didn't send enough previews. This is a bug, please report");
		}
	}
	socket->disconnectFromServer();
	emit finished();
}

void IPCPreviewClient::stopGeneration()
{
	if(!connect() || !socket->isOpen())
	{
		emit error("Could not connect to IPC worker");
		return;
	}
	QDataStream stream(socket);
	stream << StopGeneratePreviews;
	socket->flush();
}
