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
	QScopedPointer<QLocalSocket> socket{this->spawningServer.nextPendingConnection()};
	if(!socket.isNull())
	{
		if(!socket->waitForReadyRead())
		{
			return;
		}
		QDataStream stream(socket.get());
		IPCCommand command;
		stream >> command;
		if(command == GeneratePreviews)
		{
			RenderConfig renderConfig;
			QVector<RenderTarget> targets;
			stream >> renderConfig;
			stream >> targets;
		}
	}
}
