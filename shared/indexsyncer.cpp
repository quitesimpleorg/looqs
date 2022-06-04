#include <QDateTime>
#include "filesaver.h"
#include "indexsyncer.h"

IndexSyncer::IndexSyncer(SqliteDbService &dbService)
{
	this->dbService = &dbService;
}

void IndexSyncer::setDryRun(bool dryRun)
{
	this->dryRun = dryRun;
}

void IndexSyncer::setVerbose(bool verbose)
{
	this->verbose = verbose;
}

void IndexSyncer::setKeepGoing(bool keepGoing)
{
	this->keepGoing = keepGoing;
}

void IndexSyncer::setRemoveDeletedFromIndex(bool removeDeletedFromIndex)
{
	this->removeDeletedFromIndex = removeDeletedFromIndex;
}

void IndexSyncer::setPattern(QString pattern)
{
	this->pattern = pattern;
}

void IndexSyncer::sync()
{
	this->stopToken.store(false, std::memory_order_relaxed);
	FileSaver saver(*this->dbService);
	QVector<FileData> files;
	int offset = 0;
	int limit = 10000;
	unsigned int processedRows = dbService->getFiles(files, pattern, offset, limit);

	unsigned int totalUpdatesFilesCount = 0;
	unsigned int totalDeletedFilesCount = 0;
	unsigned int totalErroredFilesCount = 0;

	while(processedRows > 0)
	{
		QVector<QString> filePathsToUpdate;
		for(FileData &fileData : files)
		{
			if(processedRows % 100 == 0 && this->stopToken.load(std::memory_order_relaxed))
			{
				emit finished(totalUpdatesFilesCount, totalDeletedFilesCount, totalErroredFilesCount);
				return;
			}
			QFileInfo fileInfo(fileData.absPath);
			if(fileInfo.exists())
			{
				if(fileInfo.isFile())
				{
					if(fileInfo.lastModified().toSecsSinceEpoch() != fileData.mtime)
					{
						if(!dryRun)
						{
							filePathsToUpdate.append(fileData.absPath);
						}
						else
						{
							emit updatedDryRun(fileData.absPath);
						}
					}
				}
			}
			else
			{
				if(this->removeDeletedFromIndex)
				{
					if(!dryRun)
					{
						if(!this->dbService->deleteFile(fileData.absPath))
						{
							emit error("Error: Failed to delete " + fileData.absPath + " from the index");
							if(!this->keepGoing)
							{
								emit finished(totalUpdatesFilesCount, totalDeletedFilesCount, totalErroredFilesCount);
								return;
							}
						}
						emit removed(fileData.absPath);
						++totalDeletedFilesCount;
					}
					else
					{
						emit removedDryRun(fileData.absPath);
					}
				}
			}
		}

		unsigned int updatedFilesCount = saver.updateFiles(filePathsToUpdate, keepGoing, verbose);
		unsigned int shouldHaveUpdatedCount = static_cast<unsigned int>(filePathsToUpdate.size());
		if(updatedFilesCount != shouldHaveUpdatedCount)
		{

			totalErroredFilesCount += (shouldHaveUpdatedCount - updatedFilesCount);
			if(!keepGoing)
			{
				QString errorMsg = QString("Failed to update all files selected for updating in this batch. Updated") +
								   updatedFilesCount + "out of" + shouldHaveUpdatedCount + "selected for updating";
				emit error(errorMsg);
				emit finished(totalUpdatesFilesCount, totalDeletedFilesCount, totalErroredFilesCount);
			}
		}
		offset += limit;
		files.clear();
		totalUpdatesFilesCount += updatedFilesCount;

		processedRows = this->dbService->getFiles(files, pattern, offset, limit);
	}

	emit finished(totalUpdatesFilesCount, totalDeletedFilesCount, totalErroredFilesCount);
}

void IndexSyncer::cancel()
{
	this->stopToken.store(true, std::memory_order_seq_cst);
}
