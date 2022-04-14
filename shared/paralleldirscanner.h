#ifndef PARALLELDIRSCANNER_H
#define PARALLELDIRSCANNER_H

#include <QObject>
#include <QMutex>
#include <atomic>
#include <QThreadPool>
#include "concurrentqueue.h"
class ParallelDirScanner : public QObject
{
	Q_OBJECT
  protected:
	QStringList ignorePatterns;
	QThreadPool threadpool;

	unsigned int finishedWorkers = 0;
	unsigned int processedPaths = 0;

	std::atomic<bool> stopToken;

	bool running = false;

	QVector<QString> paths;
	ConcurrentQueue<QString> targetPathsQueue;
	ConcurrentQueue<QString> resultPathsQueue;
	unsigned int getThreadsNum() const;

  public:
	ParallelDirScanner();

	ConcurrentQueue<QString> &getResults();
	void setIgnorePatterns(QStringList patterns);
	void setPaths(QVector<QString> paths);
	void scan();
	bool isRunning();

	unsigned int pathCount();

  signals:
	void scanComplete();
	void progress(int, int);
  public slots:
	void cancel();
	void handleWorkersProgress(unsigned int progress);
	void handleWorkersFinish();
};

#endif // PARALLELDIRSCANNER_H
