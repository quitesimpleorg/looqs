#include "paralleldirscanner.h"

#include <QRunnable>
#include <QMutex>
#include <QDirIterator>
#include <QThread>
#include <QThreadPool>
#include <functional>
#include "dirscanworker.h"
#include "logger.h"

ParallelDirScanner::ParallelDirScanner()
{
	this->threadpool.setMaxThreadCount(QThread::idealThreadCount() / 2);
}

ConcurrentQueue<QString> &ParallelDirScanner::getResults()
{
	return this->resultPathsQueue;
}

void ParallelDirScanner::setIgnorePatterns(QStringList patterns)
{
	this->ignorePatterns = patterns;
}

void ParallelDirScanner::setPaths(QVector<QString> paths)
{
	this->paths = paths;
}

void ParallelDirScanner::cancel()
{
	this->stopToken.store(true, std::memory_order_relaxed);
}

void ParallelDirScanner::handleWorkersProgress(unsigned int progress)
{
	this->processedPaths += progress;
	emit this->progress(progress, this->processedPaths);
}

void ParallelDirScanner::handleWorkersFinish()
{
	Logger::info() << "Worker finished";
	// no mutexes required due to queued connection
	++finishedWorkers;
	if(finishedWorkers == getThreadsNum())
	{
		running = false;
		emit scanComplete();
	}
}

unsigned int ParallelDirScanner::getThreadsNum() const
{
	int threadsNum = this->threadpool.maxThreadCount();
	if(threadsNum > this->paths.size())
	{
		threadsNum = this->paths.size();
	}
	return threadsNum;
}

void ParallelDirScanner::scan()
{
	Logger::info() << "I am scanning";
	this->stopToken.store(false, std::memory_order_relaxed);
	this->finishedWorkers = 0;
	this->processedPaths = 0;
	this->targetPathsQueue.clear();
	this->resultPathsQueue.clear();

	this->targetPathsQueue.enqueue(this->paths);
	int threadsNum = getThreadsNum();
	if(threadsNum == 0)
	{
		emit scanComplete();
		return;
	}
	running = true;
	for(int i = 0; i < threadsNum; i++)
	{
		DirScanWorker *runnable = new DirScanWorker(this->targetPathsQueue, this->resultPathsQueue,
													this->ignorePatterns, 1000, this->stopToken);
		runnable->setAutoDelete(false);
		connect(runnable, &DirScanWorker::progress, this, &ParallelDirScanner::handleWorkersProgress,
				Qt::QueuedConnection);
		connect(runnable, &DirScanWorker::finished, this, &ParallelDirScanner::handleWorkersFinish,
				Qt::QueuedConnection);
		threadpool.start(runnable);
	}
}

bool ParallelDirScanner::isRunning()
{
	return this->running;
}

unsigned int ParallelDirScanner::pathCount()
{
	return this->processedPaths;
}
