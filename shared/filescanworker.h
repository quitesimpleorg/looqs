#ifndef FILESCANWORKER_H
#define FILESCANWORKER_H
#include <QString>
#include <QObject>
#include <QtConcurrent>
#include <utility>
#include "paralleldirscanner.h"
#include "filesaver.h"

typedef std::pair<QString, SaveFileResult> FileScanResult;

class FileScanWorker : public QObject, public QRunnable
{
	Q_OBJECT
  protected:
	SqliteDbService *dbService;
	ConcurrentQueue<QString> *queue;
	FileSaverOptions fileSaverOptions;
	int batchsize;
	std::atomic<bool> *stopToken;

  public:
	FileScanWorker(SqliteDbService &db, ConcurrentQueue<QString> &queue, int batchsize, std::atomic<bool> &stopToken);
	void run() override;
	void setFileSaverOptions(FileSaverOptions options);
  signals:
	void result(FileScanResult);
	void finished();
};

#endif // FILESCANWORKER_H
