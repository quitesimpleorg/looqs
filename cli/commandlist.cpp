#include <QCommandLineParser>
#include "commandlist.h"
#include "databasefactory.h"
#include "logger.h"

int CommandList::handle(QStringList arguments)
{
	QCommandLineParser parser;
	parser.addOptions({
		{"pattern", "Only list files from index matching the pattern, e. g. '*.txt'", "pattern"},
	});

	parser.addHelpOption();
	parser.addPositionalArgument("list", "Lists paths in the index", "list [options]");

	parser.process(arguments);

	QString pattern = parser.value("pattern");

	QVector<FileData> results;

	int offset = 0;
	int limit = 1000;

	auto resultscount = dbService->getFiles(results, pattern, offset, limit);
	while(resultscount > 0)
	{
		for(FileData &fileData : results)
		{
			Logger::info() << fileData.absPath << Qt::endl;
		}
		offset += limit;
		results.clear();
		resultscount = dbService->getFiles(results, pattern, offset, limit);
	}
	return 0;
}
