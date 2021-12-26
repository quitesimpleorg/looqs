#include <QFile>
#include <QDesktopServices>
#include <QSettings>
#include <QProcess>
#include <QUrl>
#include <QLocalSocket>
#include <QDataStream>
#include "ipcserver.h"

IpcServer::IpcServer()
{
	connect(&this->spawningServer, &QLocalServer::newConnection, this, &IpcServer::spawnerNewConnection);
}

bool IpcServer::startSpawner(QString socketPath)
{
	QFile::remove(socketPath);
	return this->spawningServer.listen(socketPath);
}

bool IpcServer::docOpen(QString path, int pagenum)
{
	QSettings settings;
	QString command = settings.value("pdfviewer").toString();
	if(command != "" && command.contains("%p") && command.contains("%f"))
	{
		QStringList splitted = command.split(" ");
		if(splitted.size() > 1)
		{
			QString cmd = splitted[0];
			QStringList args = splitted.mid(1);
			args.replaceInStrings("%f", path);
			args.replaceInStrings("%p", QString::number(pagenum));

			QProcess::startDetached(cmd, args);
		}
	}
	else
	{
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}
	return true;
}

bool IpcServer::fileOpen(QString path)
{
	return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
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
		QStringList args;
		stream >> command;
		stream >> args;
		if(args.size() < 1)
		{
			stream << "invalid";
			return;
		}
		if(command == DocOpen)
		{
			if(args.size() < 2)
			{
				stream << "invalid";
				return;
			}
			docOpen(args[0], args[1].toInt());
		}
		if(command == FileOpen)
		{
			if(args.size() < 1)
			{
				stream << "invalid";
				return;
			}
			fileOpen(args[0]);
		}
	}
}
