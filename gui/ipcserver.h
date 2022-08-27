#ifndef IPCSERVER_H
#define IPCSERVER_H
#include <QString>
#include <QLocalServer>
#include "ipc.h"
#include "filesaver.h"
#include "ipcpreviewworker.h"

class IpcServer : public QObject
{
	Q_OBJECT
  private:
	QLocalServer spawningServer;
	SaveFileResult addFile(QString file);
  private slots:
	void spawnerNewConnection();

  public:
	IpcServer();
	bool startSpawner(QString socketPath);
};

#endif // IPCSERVER_H
