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

	int ret = 0;
	if(!keepGoing && failedPathsCount > 0)
	{
		ret = 1;
	}
	emit finishedCmd(ret);
}

int CommandAdd::handle(QStringList arguments)
{
	QCommandLineParser parser;
	parser.addOptions({{{"c", "continue"},
						"Continue adding files, don't exit on first error. If this option is not given, looqs will "
						"exit asap, but it's possible that a few files will still be processed. "
						"Set -t 1 to avoid this behavior, but processing will be slower. "},
					   {{"n", "no-content"}, "Only add paths to database. Do not index content"},
					   {{"f", "fill-content"}, "Index content for files previously indexed with -n"},
					   {"tags", "Comma-separated list of tags to assign"},
					   {{"t", "threads"}, "Number of threads to use.", "threads"}});
	parser.addHelpOption();
	parser.addPositionalArgument("add", "Add paths to the index",
								 "add [paths...]. If no path is given, read from stdin, one path per line.");

	parser.process(arguments);
	this->keepGoing = parser.isSet("continue");
	bool pathsOnly = parser.isSet("no-content");
	bool fillContent = parser.isSet("fill-content");
	if(parser.isSet("threads"))
	{
		QString threadsCount = parser.value("threads");
		QThreadPool::globalInstance()->setMaxThreadCount(threadsCount.toInt());
	}

	if(pathsOnly && fillContent)
	{
		Logger::error() << "Invalid options: -n and -f cannot both be set";
		return EXIT_FAILURE;
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

	FileSaverOptions fileSaverOptions;
	fileSaverOptions.keepGoing = keepGoing;
	fileSaverOptions.fillPathsOnlyWithContent = fillContent;
	fileSaverOptions.pathsOnly = pathsOnly;
	fileSaverOptions.verbose = false;

	indexer = new Indexer(*this->dbService);
	indexer->setFileSaverOptions(fileSaverOptions);

	indexer->setTargetPaths(files.toVector());

	connect(indexer, &Indexer::pathsCountChanged, this,
			[](int pathsCount) { Logger::info() << "Found paths: " << pathsCount << Qt::endl; });
	connect(indexer, &Indexer::indexProgress, this,
			[](int pathsCount, unsigned int added, unsigned int skipped, unsigned int failed, unsigned int totalCount)
			{ Logger::info() << "Processed files: " << pathsCount << Qt::endl; });
	connect(indexer, &Indexer::finished, this, &CommandAdd::indexerFinished);

	this->autoFinish = false;
	indexer->beginIndexing();

	return 0;
}
