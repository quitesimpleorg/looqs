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
	auto paths = queue->dequeue(batchsize);
	for(QString &path : paths)
	{
		SaveFileResult sfr;
		try
		{
			sfr = saver.addFile(path);
		}
		catch(std::runtime_error &e)
		{
			Logger::error() << e.what();
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
