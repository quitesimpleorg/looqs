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
	IPCPreviewWorker previewWorker;
	QLocalServer spawningServer;
	QLocalSocket *currentSocket = nullptr;
	SaveFileResult addFile(QString file);
  private slots:
	void spawnerNewConnection();
	void handlePreviewGenerated(QByteArray ba);

  public:
	IpcServer();
	bool startSpawner(QString socketPath);
};

#endif // IPCSERVER_H
