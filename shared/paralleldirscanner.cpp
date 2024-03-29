#include <QRunnable>
#include <QMutex>
#include <QDirIterator>
#include <QThread>
#include <QThreadPool>
#include <functional>
#include "dirscanworker.h"
#include "paralleldirscanner.h"

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
	this->stopToken.store(true, std::memory_order_seq_cst);
}

void ParallelDirScanner::handleWorkersProgress(unsigned int progress)
{
	this->processedPaths += progress;
	if(!this->stopToken.load(std::memory_order_seq_cst))
		emit this->progress(progress, this->processedPaths);
}

void ParallelDirScanner::handleWorkersFinish()
{
	// no mutexes required due to queued connection
	++finishedWorkers;
	if(this->stopToken.load(std::memory_order_seq_cst) || finishedWorkers == getThreadsNum())
	{
		running = false;
		emit scanComplete();
	}
	delete QObject::sender();
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
	this->stopToken.store(false, std::memory_order_relaxed);
	this->finishedWorkers = 0;
	this->processedPaths = 0;
	this->targetPathsQueue.clear();
	this->resultPathsQueue.clear();

	/* First scan without subdirs. This way we collect paths for the threads */
	WildcardMatcher matcher;
	matcher.setPatterns(this->ignorePatterns);
	for(QString &path : this->paths)
	{
		QDirIterator iterator(path, QStringList{}, QDir::Dirs | QDir::QDir::Files | QDir::NoDotAndDotDot);
		while(iterator.hasNext())
		{
			QString path = iterator.next();
			if(matcher.match(path))
			{
				continue;
			}
			QFileInfo info = iterator.fileInfo();
			if(!info.isSymLink())
			{
				if(info.isDir())
				{
					this->targetPathsQueue.enqueue(info.absoluteFilePath());
				}
				else
				{
					this->resultPathsQueue.enqueue(info.absoluteFilePath());
					this->processedPaths += 1;
				}
			}
		}
	}
	emit this->progress(this->processedPaths, this->processedPaths);
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
