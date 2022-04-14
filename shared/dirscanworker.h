#ifndef DIRSCANWORKER_H
#define DIRSCANWORKER_H
#include <QObject>
#include <QRunnable>
#include <QDirIterator>
#include "concurrentqueue.h"
class DirScanWorker : public QObject, public QRunnable
{
	Q_OBJECT
  protected:
	unsigned int progressReportThreshold = 1000;
	ConcurrentQueue<QString> *queue = nullptr;
	ConcurrentQueue<QString> *resultQueue = nullptr;

	QStringList ignorePattern;
	QVector<QString> results;

	std::atomic<bool> *stopToken;

  public:
	DirScanWorker(ConcurrentQueue<QString> &queue, ConcurrentQueue<QString> &resultQueue, QStringList ignorePattern,
				  unsigned int progressReportThreshold, std::atomic<bool> &stopToken);

	void run() override;

  signals:
	void progress(unsigned int);
	void finished();
};

#endif // DIRSCANWORKER_H
