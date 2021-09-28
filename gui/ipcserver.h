#ifndef IPCSERVER_H
#define IPCSERVER_H
#include <QString>
#include <QLocalServer>
#include "ipc.h"
class IpcServer : public QObject
{
	Q_OBJECT
  private:
	QLocalServer spawningServer;
	bool docOpen(QString path, int pagenum);
	bool fileOpen(QString path);
  private slots:
	void spawnerNewConnection();

  public:
	IpcServer();
	bool startSpawner(QString socketPath);
};

#endif // IPCSERVER_H
