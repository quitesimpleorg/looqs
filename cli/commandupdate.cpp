#include <QCommandLineParser>
#include <QFileInfo>
#include <QDateTime>
#include <QThreadPool>
#include "commandupdate.h"
#include "logger.h"
#include "../shared/indexsyncer.h"

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

	bool hasErrors = false;
	IndexSyncer *syncer = new IndexSyncer(*this->dbService);
	syncer->setKeepGoing(keepGoing);
	syncer->setVerbose(verbose);
	syncer->setPattern(pattern);
	syncer->setDryRun(dryRun);
	syncer->setRemoveDeletedFromIndex(deleteMissing);

	if(dryRun)
	{
		connect(syncer, &IndexSyncer::removedDryRun, this,
				[](QString path) { Logger::info() << "Would delete" << path << Qt::endl; });
		connect(syncer, &IndexSyncer::updatedDryRun, this,
				[](QString path) { Logger::info() << "Would update" << path << Qt::endl; });
	}
	else
	{
		connect(syncer, &IndexSyncer::removed, this,
				[](QString path) { Logger::info() << "Removed " << path << Qt::endl; });
		/* TODO: updated not printed, handled be verbose in FileSaver, but this can be improved */
	}
	connect(syncer, &IndexSyncer::finished, this,
			[&](unsigned int totalUpdated, unsigned int totalRemoved, unsigned int totalErrors)
			{
				Logger::info() << "Syncing finished" << Qt::endl;

				if(!dryRun)
				{
					Logger::info() << "Total updated:" << totalUpdated << Qt::endl;
					Logger::info() << "Total removed from index: " << totalRemoved << Qt::endl;
					Logger::info() << "Total deleted:" << totalErrors << Qt::endl;
				}

				int retval = 0;
				if(hasErrors && !keepGoing)
				{
					retval = 1;
				}
				emit finishedCmd(retval);
			});
	connect(syncer, &IndexSyncer::error, this,
			[&](QString error)
			{
				Logger::error() << error << Qt::endl;
				hasErrors = true;
			});

	this->autoFinish = false;
	syncer->sync();
	/* Actual return code is handled by finishedCmd signal */
	return 0;
}
