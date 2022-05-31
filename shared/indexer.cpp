#include "indexer.h"
#include "logger.h"
#include "wildcardmatcher.h"

Indexer::Indexer(SqliteDbService &db)
{
	dirScanner = QSharedPointer<ParallelDirScanner>(new ParallelDirScanner());
	connect(dirScanner.data(), &ParallelDirScanner::scanComplete, this, &Indexer::dirScanFinished);
	connect(dirScanner.data(), &ParallelDirScanner::progress, this, &Indexer::dirScanProgress);
	this->db = &db;
}

void Indexer::beginIndexing()
{
	this->runningWorkers = 0;
	this->currentScanProcessedCount = 0;
	this->currentIndexResult = IndexResult();
	this->currentIndexResult.begin = QDateTime::currentDateTime();
	QVector<QString> dirs;

	WildcardMatcher wildcardMatcher;
	wildcardMatcher.setPatterns(this->ignorePattern);
	for(QString &path : this->pathsToScan)
	{
		if(wildcardMatcher.match(path))
		{
			continue;
		}
		QFileInfo info{path};
		if(info.isDir())
		{
			dirs.append(path);
		}
		else
		{
			this->filePathTargetsQueue.enqueue(path);
		}
	}
	this->dirScanner->setPaths(dirs);
	this->dirScanner->setIgnorePatterns(this->ignorePattern);

	this->dirScanner->scan();

	this->workerCancellationToken.store(false, std::memory_order_seq_cst);
	launchWorker(this->filePathTargetsQueue, this->filePathTargetsQueue.remaining());
}

void Indexer::setIgnorePattern(QStringList ignorePattern)
{
	this->ignorePattern = ignorePattern;
}

void Indexer::setTargetPaths(QVector<QString> pathsToScan)
{
	this->pathsToScan = pathsToScan;
}

void Indexer::requestCancellation()
{
	this->dirScanner->cancel();
	this->workerCancellationToken.store(true, std::memory_order_release);
}

IndexResult Indexer::getResult()
{
	return this->currentIndexResult;
}

void Indexer::dirScanFinished()
{
	Logger::info() << "Dir scan finished";
	if(!isRunning())
	{
		emit finished();
	}
}

void Indexer::launchWorker(ConcurrentQueue<QString> &queue, int batchsize)
{
	FileScanWorker *runnable = new FileScanWorker(*this->db, queue, batchsize, this->workerCancellationToken);
	connect(runnable, &FileScanWorker::result, this, &Indexer::processFileScanResult);
	connect(runnable, &FileScanWorker::finished, this, &Indexer::processFinishedWorker);
	++this->runningWorkers;
	QThreadPool::globalInstance()->start(runnable);
}

void Indexer::dirScanProgress(int current, int total)
{
	launchWorker(this->dirScanner->getResults(), current);
	emit pathsCountChanged(total);
}

void Indexer::processFileScanResult(FileScanResult result)
{
	if(verbose)
	{
		this->currentIndexResult.results.append(result);
	}
	else
	{
		if(result.second == DBFAIL || result.second == PROCESSFAIL || result.second == NOTFOUND)
		{
			this->currentIndexResult.results.append(result);
		}
	}
	if(result.second == OK)
	{
		++this->currentIndexResult.addedPaths;
	}
	else if(result.second == SKIPPED)
	{
		++this->currentIndexResult.skippedPaths;
	}
	else
	{
		++this->currentIndexResult.erroredPaths;
	}

	if(currentScanProcessedCount++ == progressReportThreshold)
	{
		emit indexProgress(this->currentIndexResult.total(), this->currentIndexResult.addedPaths,
						   this->currentIndexResult.skippedPaths, this->currentIndexResult.erroredPaths,
						   this->dirScanner->pathCount());
		currentScanProcessedCount = 0;
	}
}

bool Indexer::isRunning()
{
	return this->runningWorkers > 0 || this->dirScanner->isRunning();
}
void Indexer::processFinishedWorker()
{
	--this->runningWorkers;
	if(!isRunning())
	{
		emit finished();
	}
}
