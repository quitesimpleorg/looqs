#ifndef INDEXER_H
#define INDEXER_H
#include <QVector>
#include <QObject>
#include "sqlitedbservice.h"
#include "paralleldirscanner.h"
#include "filescanworker.h"

class IndexResult
{
  public:
	QDateTime begin;
	QDateTime end;
	QVector<FileScanResult> results;

	unsigned int addedPaths = 0;
	unsigned int skippedPaths = 0;
	unsigned int erroredPaths = 0;

	unsigned int total()
	{
		return addedPaths + skippedPaths + erroredPaths;
	}

	QVector<QString> failedPaths() const
	{
		QVector<QString> result;
		std::for_each(results.begin(), results.end(),
					  [&result](FileScanResult res)
					  {
						  if(res.second == DBFAIL || res.second == PROCESSFAIL || res.second == NOTFOUND)
						  {
							  result.append(res.first);
						  }
					  });
		return result;
	}
};

class Indexer : public QObject
{
	Q_OBJECT
  protected:
	bool verbose = false;
	bool keepGoing = true;
	SqliteDbService *db;

	int progressReportThreshold = 50;
	int currentScanProcessedCount = 0;
	int runningWorkers = 0;

	QVector<QString> pathsToScan;
	QSharedPointer<ParallelDirScanner> dirScanner;
	QSharedPointer<FileScanWorker> fileScanner;

	QStringList ignorePattern;

	/* Those path pointing to files not directories */
	ConcurrentQueue<QString> filePathTargetsQueue;

	IndexResult currentIndexResult;
	void launchWorker(ConcurrentQueue<QString> &queue, int batchsize);

  public:
	void beginIndexing();
	void setIgnorePattern(QStringList ignorePattern);
	void setTargetPaths(QVector<QString> pathsToScan);

	Indexer(SqliteDbService &db);
	IndexResult getResult();

  public slots:
	void dirScanFinished();
	void dirScanProgress(int current, int total);
	void processFileScanResult(FileScanResult result);
	void processFinishedWorker();

  signals:
	void pathsCountChanged(int total);
	void fileScanResult(FileScanResult *result);
	void indexProgress(unsigned int processedFiles, unsigned int added, unsigned int skipped, unsigned int failed,
					   unsigned int totalPaths);
	void finished();
};

#endif // INDEXER_H
