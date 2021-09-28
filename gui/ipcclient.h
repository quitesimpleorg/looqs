#ifndef IPCCLIENT_H
#define IPCCLIENT_H
#include <QLocalSocket>
#include <QString>
#include <QStringList>
#include "ipc.h"

class IPCClient
{
  private:
	QString socketPath;

  public:
	IPCClient(QString socketPath);
	bool sendCommand(IPCCommand command, QStringList args);
};

#endif // IPCCLIENT_H
