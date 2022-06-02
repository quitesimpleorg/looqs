#include <QCommandLineParser>
#include <QFileInfo>
#include <QDateTime>
#include <QThreadPool>
#include "commandupdate.h"
#include "logger.h"

int CommandUpdate::handle(QStringList arguments)
{
	QCommandLineParser parser;
	parser.addOptions(
		{{{"v", "verbose"}, "Print path of the files while updating them"},
		 {{"n", "dry-run"}, "Only print which files would be updated, don't actually update them"},
		 {"pattern", "Only consider to update files in the index matching the pattern, e. g. */.git/*.", "pattern"},
		 {{"d", "delete"}, "If a file does not exist anymore, delete it from the index"},
		 {{"c", "continue"},
		  "Continue adding files, don't exit on first error. If this option is not given, looqs will exit asap, but "
		  "it's possible that a few files will still be processed. "
		  "Set -t 1 to avoid this behavior, but processing will be slower."},
		 {{"t", "threads"}, "Number of threads to use.", "threads"}

		});

	parser.addHelpOption();
	parser.addPositionalArgument("update", "Checks files for changes and updates the index", "update");

	parser.process(arguments);
	bool keepGoing = parser.isSet("continue");
	bool verbose = parser.isSet("verbose");
	bool deleteMissing = parser.isSet("delete");
	bool dryRun = parser.isSet("dry-run");
	QString pattern = parser.value("pattern");

	if(parser.isSet("threads"))
	{
		QString threadsCount = parser.value("threads");
		QThreadPool::globalInstance()->setMaxThreadCount(threadsCount.toInt());
	}
	FileSaver saver(*this->dbService);
	QVector<FileData> files;
	int offset = 0;
	int limit = 1000;
	int processedRows = dbService->getFiles(files, pattern, offset, limit);

	unsigned int totalUpdatesFilesCount = 0;
	unsigned int totalDeletedFilesCount = 0;

	while(processedRows > 0)
	{
		QVector<QString> filePathsToUpdate;
		for(FileData &fileData : files)
		{
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
							Logger::info() << "Would update" << fileData.absPath << Qt::endl;
						}
					}
				}
			}
			else
			{
				if(deleteMissing)
				{
					if(!dryRun)
					{
						if(!this->dbService->deleteFile(fileData.absPath))
						{
							Logger::error()
								<< "Error: Failed to delete" << fileData.absPath << "from the index" << Qt::endl;
							if(!keepGoing)
							{
								return 1;
							}
						}
						if(verbose)
						{
							Logger::info() << "Deleted from index:" << fileData.absPath << Qt::endl;
						}
						++totalDeletedFilesCount;
					}
					else
					{

						Logger::info() << "Would delete from index" << fileData.absPath << Qt::endl;
					}
				}
			}
		}

		int updatedFilesCount = saver.updateFiles(filePathsToUpdate, keepGoing, verbose);
		int shouldHaveUpdatedCount = filePathsToUpdate.size();
		if(updatedFilesCount != shouldHaveUpdatedCount)
		{
			if(!keepGoing)
			{
				Logger::error() << "Failed to update all files selected for updating in this batch. Updated"
								<< updatedFilesCount << "out of" << shouldHaveUpdatedCount << "selected for updating"
								<< Qt::endl;
				return 1;
			}
		}
		offset += limit;
		files.clear();
		processedRows = this->dbService->getFiles(files, pattern, offset, limit);

		totalUpdatesFilesCount += static_cast<unsigned int>(updatedFilesCount);
	}
	if(!dryRun)
	{
		Logger::info() << "Total (updated): " << totalUpdatesFilesCount << Qt::endl;
		Logger::info() << "Total (deleted from index): " << totalDeletedFilesCount << Qt::endl;
	}

	return 0;
}
