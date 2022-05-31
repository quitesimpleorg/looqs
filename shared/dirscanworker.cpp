#include <QThread>
#include "dirscanworker.h"
#include "logger.h"
DirScanWorker::DirScanWorker(ConcurrentQueue<QString> &queue, ConcurrentQueue<QString> &resultQueue,
							 QStringList ignorePattern, unsigned int progressReportThreshold,
							 std::atomic<bool> &stopToken)
{
	this->queue = &queue;
	this->resultQueue = &resultQueue;
	this->wildcardMatcher.setPatterns(ignorePattern);
	this->progressReportThreshold = progressReportThreshold;
	this->stopToken = &stopToken;
	setAutoDelete(false);
}

void DirScanWorker::run()
{
	unsigned int currentProgress = 0;
	QString path;
	/* TODO: if we have e. g. only one path, then only one thread will scan this path.
		 *
		 * Thus, we must resubmit to the queue directories so other threads can help
		the current one (requires a new logic for threads in ParallelDirScanner). Alterantively,
		start new DirScanWorkers ourselves here... */
	while(queue->dequeue(path))
	{
		if(wildcardMatcher.match(path))
		{
			continue;
		}
		QDirIterator iterator(path, QStringList{}, QDir::Files, QDirIterator::Subdirectories);
		while(iterator.hasNext())
		{
			QString entry = iterator.next();
			if(wildcardMatcher.match(entry))
			{
				continue;
			}
			this->results.append(entry);
			++currentProgress;
			if(currentProgress == progressReportThreshold)
			{
				if(this->stopToken->load(std::memory_order_relaxed))
				{
					Logger::info() << "Received cancel request" << Qt::endl;
					this->results.clear();
					emit finished();
					return;
				}

				this->resultQueue->enqueue(this->results);
				emit progress(results.length());
				currentProgress = 0;
				this->results.clear();
			}
		}
	}
	this->resultQueue->enqueue(this->results);
	emit progress(results.length());
	this->results.clear();
	emit finished();
}
