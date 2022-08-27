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
}

bool IpcServer::startSpawner(QString socketPath)
{
	QFile::remove(socketPath);
	return this->spawningServer.listen(socketPath);
}

void IpcServer::spawnerNewConnection()
{
	QLocalSocket *socket = this->spawningServer.nextPendingConnection();
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
			if(socket->state() == QLocalSocket::ConnectedState)
			{
				stream << targets.count();
				socket->flush();
				IPCPreviewWorker *previewWorker = new IPCPreviewWorker(socket);
				connect(previewWorker, &IPCPreviewWorker::finished, this, [previewWorker] { delete previewWorker; });
				previewWorker->start(renderConfig, targets);
			}
			else
			{
				delete socket;
			}
		}
		if(command == StopGeneratePreviews)
		{
			/* TODO: implement */
		}
	}
}
