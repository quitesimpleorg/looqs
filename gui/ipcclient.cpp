#include <QDataStream>
#include "ipcclient.h"

IPCClient::IPCClient(QString socketPath)
{
	this->socketPath = socketPath;
}

bool IPCClient::sendCommand(IPCCommand command, QStringList args)
{
	bool result = false;
	QLocalSocket socket;
	socket.connectToServer(socketPath);
	if(socket.isOpen() && socket.isWritable())
	{
		QDataStream stream(&socket);
		stream << command;
		stream << args;
		socket.flush();
		result = true;
	}
	else
	{
		qDebug() << "Not connected to IPC server";
	}
	return result;
}
