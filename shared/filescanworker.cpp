#include "filescanworker.h"
#include "logger.h"
FileScanWorker::FileScanWorker(SqliteDbService &db, ConcurrentQueue<QString> &queue, int batchsize,
							   std::atomic<bool> &stopToken)
{
	this->dbService = &db;
	this->queue = &queue;
	this->batchsize = batchsize;
	this->stopToken = &stopToken;
}

void FileScanWorker::run()
{
	FileSaver saver{*this->dbService};
	saver.setFileSaverOptions(this->fileSaverOptions);
	auto paths = queue->dequeue(batchsize);
	for(QString &path : paths)
	{
		SaveFileResult sfr;
		try
		{
			sfr = saver.addFile(path);
		}
		catch(LooqsGeneralException &e)
		{
			Logger::error() << e.message << Qt::endl;
			sfr = PROCESSFAIL;
		}

		catch(std::exception &e)
		{
			Logger::error() << e.what() << Qt::endl;
			sfr = PROCESSFAIL; // well...
		}

		emit result({path, sfr});
		if(stopToken->load(std::memory_order_relaxed)) // TODO: relaxed should suffice here, but recheck
		{
			emit finished();
			return;
		}
	}
	emit finished();
}

void FileScanWorker::setFileSaverOptions(FileSaverOptions options)
{
	this->fileSaverOptions = options;
}
