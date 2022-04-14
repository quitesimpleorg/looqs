#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QTextStream>
#include <QException>
#include <QCommandLineParser>
#include <QMutex>
#include <QMutexLocker>
#include <QtConcurrent/QtConcurrentMap>
#include "commandadd.h"
#include "logger.h"

void CommandAdd::indexerFinished()
{
	IndexResult result = indexer->getResult();

	Logger::info() << "Total: " << result.total() << Qt::endl;
	Logger::info() << "Added: " << result.addedPaths << Qt::endl;
	Logger::info() << "Skipped: " << result.skippedPaths << Qt::endl;
	auto failedPathsCount = result.erroredPaths;
	Logger::info() << "Failed: " << failedPathsCount << Qt::endl;
	if(failedPathsCount > 0)
	{
		Logger::info() << "Failed paths: " << Qt::endl;
		for(QString paths : result.failedPaths())
		{
			Logger::info() << paths << Qt::endl;
		}
	}

	/* TODO maybe not 0 if keepGoing not given */
	emit finishedCmd(0);
}

int CommandAdd::handle(QStringList arguments)
{
	QCommandLineParser parser;
	parser.addOptions({{{"c", "continue"},
						"Continue adding files, don't exit on first error. If this option is not given, looqs will "
						"exit asap, but it's possible that a few files will still be processed. "
						"Set -t 1 to avoid this behavior, but processing will be slower. "},
					   {{"a", "all"}, "On error, no files should be added, even already processed ones"},
					   {{"v", "verbose"}, "Print skipped and added files"},
					   {{"t", "threads"}, "Number of threads to use.", "threads"}});

	parser.addHelpOption();
	parser.addPositionalArgument("add", "Add paths to the index", "add [paths...]");

	parser.process(arguments);
	bool keepGoing = parser.isSet("continue");
	bool verbose = parser.isSet("verbose");
	if(parser.isSet("all"))
	{
		throw LooqsGeneralException("To be implemented");
	}
	if(parser.isSet("threads"))
	{
		QString threadsCount = parser.value("threads");
		QThreadPool::globalInstance()->setMaxThreadCount(threadsCount.toInt());
	}

	QStringList files = parser.positionalArguments();

	if(files.length() == 0)
	{
		QTextStream stream(stdin);

		while(!stream.atEnd())
		{
			QString path = stream.readLine();
			files.append(path);
		}
	}

	indexer = new Indexer(*this->dbService);
	indexer->setTargetPaths(files.toVector());

	connect(indexer, &Indexer::pathsCountChanged, this,
			[](int pathsCount) { Logger::info() << "Found paths: " << pathsCount << Qt::endl; });

	connect(indexer, &Indexer::indexProgress, this,
			[](int pathsCount, unsigned int added, unsigned int skipped, unsigned int failed, unsigned int totalCount)
			{ Logger::info() << "Processed files: " << totalCount << Qt::endl; });
	connect(indexer, &Indexer::finished, this, &CommandAdd::indexerFinished);

	/* TODO: keepGoing, verbose */

	this->autoFinish = false;
	indexer->beginIndexing();

	return 0;
}
