#ifndef IPCSERVER_H
#define IPCSERVER_H
#include <QString>
#include <QLocalServer>
#include "ipc.h"
#include "filesaver.h"
class IpcServer : public QObject
{
	Q_OBJECT
  private:
	QSharedPointer<DatabaseFactory> dbFactory;
	QSharedPointer<SqliteDbService> dbService;
	QSharedPointer<FileSaver> fileSaver;
	QLocalServer spawningServer;
	bool docOpen(QString path, int pagenum);
	bool fileOpen(QString path);
	SaveFileResult addFile(QString file);
  private slots:
	void spawnerNewConnection();

  public:
	IpcServer();
	bool startSpawner(QString socketPath);
};

#endif // IPCSERVER_H
