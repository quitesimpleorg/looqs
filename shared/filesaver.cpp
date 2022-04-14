#include <QSqlError>
#include <QDateTime>
#include <QtConcurrentMap>
#include <QProcess>
#include <functional>
#include "filesaver.h"
#include "processor.h"
#include "pdfprocessor.h"
#include "defaulttextprocessor.h"
#include "tagstripperprocessor.h"
#include "nothingprocessor.h"
#include "odtprocessor.h"
#include "odsprocessor.h"
#include "utils.h"
#include "logger.h"

FileSaver::FileSaver(SqliteDbService &dbService)
{
	this->dbService = &dbService;
}

SaveFileResult FileSaver::addFile(QString path)
{
	QFileInfo info(path);
	QString absPath = info.absoluteFilePath();
	auto mtime = info.lastModified().toSecsSinceEpoch();
	if(this->dbService->fileExistsInDatabase(absPath, mtime))
	{
		return SKIPPED;
	}
	return saveFile(info);
}

SaveFileResult FileSaver::updateFile(QString path)
{
	QFileInfo info(path);
	return saveFile(info);
}

int FileSaver::addFiles(const QVector<QString> paths, bool keepGoing, bool verbose)
{
	return processFiles(paths, std::bind(&FileSaver::addFile, this, std::placeholders::_1), keepGoing, verbose);
}

int FileSaver::updateFiles(const QVector<QString> paths, bool keepGoing, bool verbose)
{
	return processFiles(paths, std::bind(&FileSaver::updateFile, this, std::placeholders::_1), keepGoing, verbose);
}

int FileSaver::processFiles(const QVector<QString> paths, std::function<SaveFileResult(QString path)> saverFunc,
							bool keepGoing, bool verbose)
{
	std::atomic<bool> terminate{false};
	std::atomic<int> processedCount{0};
	QtConcurrent::blockingMap(paths,
							  [&](const QString &path)
							  {
								  if(terminate.load())
								  {
									  return;
								  }
								  if(verbose)
								  {
									  Logger::info() << "Processing " << path << Qt::endl;
								  }
								  SaveFileResult result = saverFunc(path);
								  if(result == DBFAIL || result == PROCESSFAIL)
								  {
									  Logger::error() << "Failed to process " << path << Qt::endl;
									  if(!keepGoing)
									  {
										  terminate = true;
									  }
								  }
								  else
								  {
									  ++processedCount;
									  if(verbose)
									  {
										  if(result == SKIPPED)
										  {
											  Logger::info() << "Skipped" << path
															 << "as it already exists in the database" << Qt::endl;
										  }
										  else if(result == OK)
										  {
											  Logger::info() << "Added" << path << Qt::endl;
										  }
									  }
								  }
							  });
	return processedCount.load();
}

SaveFileResult FileSaver::saveFile(const QFileInfo &fileInfo)
{
	QVector<PageData> pageData;
	QString absPath = fileInfo.absoluteFilePath();

	int status = -1;

	if(!fileInfo.exists())
	{
		return NOTFOUND;
	}

	if(fileInfo.isFile())
	{
		QProcess process;
		QStringList args;
		args << "process" << absPath;
		process.setProcessChannelMode(QProcess::ForwardedErrorChannel);
		process.start("/proc/self/exe", args);
		process.waitForStarted();
		process.waitForFinished();

		/* TODO: This is suboptimal as it eats lots of mem
		 * but avoids a weird QDataStream/QProcess behaviour
		 * where it thinks the process has ended when it has not...
		 *
		 * Also, there seem to be issues with reads not being blocked, so
		 * the only reliable way appears to be waiting until the process
		 * finishes.
		 */
		QDataStream in(process.readAllStandardOutput());
		while(!in.atEnd())
		{
			PageData pd;
			in >> pd;
			pageData.append(pd);
		}
		status = process.exitCode();
		if(status != 0 && status != NOTHING_PROCESSED)
		{
			Logger::error() << "FileSaver::saveFile(): Error while processing" << absPath << ":"
							<< "Exit code " << status << Qt::endl;

			return PROCESSFAIL;
		}
	}

	// Could happen if a file corrupted for example
	if(pageData.isEmpty() && status != NOTHING_PROCESSED)
	{
		Logger::error() << "Could not get any content for " << absPath << Qt::endl;
	}

	return this->dbService->saveFile(fileInfo, pageData);
}
