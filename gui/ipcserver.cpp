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
	connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
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
			IPCPreviewWorker *worker = new IPCPreviewWorker();
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
			worker->start(renderConfig, targets, socket);
		}
	}
}
