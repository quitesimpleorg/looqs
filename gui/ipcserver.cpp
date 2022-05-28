#include <QFile>
#include <QDesktopServices>
#include <QSettings>
#include <QProcess>
#include <QUrl>
#include <QLocalSocket>
#include <QDataStream>
#include "ipcserver.h"
#include "common.h"
#include "databasefactory.h"
#include "../shared/logger.h"
#include "renderconfig.h"
#include "rendertarget.h"
#include "ipcpreviewworker.h"

IpcServer::IpcServer()
{
	/* Only 1, we are doing work for the GUI, not a service for general availability */
	this->spawningServer.setMaxPendingConnections(1);
	connect(&this->spawningServer, &QLocalServer::newConnection, this, &IpcServer::spawnerNewConnection);
	connect(&this->previewWorker, &IPCPreviewWorker::previewGenerated, this, &IpcServer::handlePreviewGenerated);
	connect(&this->previewWorker, &IPCPreviewWorker::finished, this, [this] { this->currentSocket->flush(); });
}

bool IpcServer::startSpawner(QString socketPath)
{
	QFile::remove(socketPath);
	return this->spawningServer.listen(socketPath);
}

void IpcServer::spawnerNewConnection()
{
	QLocalSocket *socket = this->spawningServer.nextPendingConnection();
	connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
	this->currentSocket = socket;
	if(socket != nullptr)
	{
		if(!socket->waitForReadyRead())
		{
			return;
		}
		QDataStream stream(socket);
		IPCCommand command;
		stream >> command;
		if(command == GeneratePreviews)
		{
			RenderConfig renderConfig;
			QVector<RenderTarget> targets;
			do
			{
				/* TODO: this is not entirely robust */
				socket->waitForReadyRead(100);
				stream.startTransaction();
				stream >> renderConfig >> targets;
			} while(!stream.commitTransaction() && socket->state() == QLocalSocket::ConnectedState);

			stream << targets.count();
			socket->flush();
			previewWorker.start(renderConfig, targets, socket);
		}
		if(command == StopGeneratePreviews)
		{
			previewWorker.stop();
		}
	}
}

void IpcServer::handlePreviewGenerated(QByteArray ba)
{
	QDataStream stream{this->currentSocket};
	stream << ba;
	this->currentSocket->flush();
}
