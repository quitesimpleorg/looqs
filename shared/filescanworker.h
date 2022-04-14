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
	int batchsize;

  public:
	FileScanWorker(SqliteDbService &db, ConcurrentQueue<QString> &queue, int batchsize);
	void run() override;
  signals:
	void result(FileScanResult);
	void finished();
};

#endif // FILESCANWORKER_H