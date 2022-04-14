#include "indexer.h"
#include "logger.h"

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

	for(QString &path : this->pathsToScan)
	{
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

IndexResult Indexer::getResult()
{
	return this->currentIndexResult;
}

void Indexer::dirScanFinished()
{
	Logger::info() << "Dir scan finished";
}

void Indexer::launchWorker(ConcurrentQueue<QString> &queue, int batchsize)
{
	FileScanWorker *runnable = new FileScanWorker(*this->db, queue, batchsize);
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

void Indexer::processFinishedWorker()
{
	--this->runningWorkers;
	if(this->runningWorkers == 0 && !this->dirScanner->isRunning())
	{
		emit finished();
	}
}
