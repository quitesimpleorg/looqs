#include <QCommandLineParser>
#include "commandsearch.h"
#include "databasefactory.h"
#include "logger.h"

int CommandSearch::handle(QStringList arguments)
{
	QCommandLineParser parser;
	parser.addOptions({
		{{"r", "reverse"},
		 "Print most-recently changed files last. This is short for adding \"sort:(mtime asc)\" to the query."},
	});

	parser.addHelpOption();
	parser.process(arguments);

	QStringList terms = parser.positionalArguments();
	if(terms.length() == 0)
	{
		Logger::error() << "Please enter search terms" << Qt::endl;
		return 1;
	}
	QString queryStrings = terms.join(' ');
	LooqsQuery query = LooqsQuery::build(queryStrings, TokenType::FILTER_PATH_CONTAINS, false);
	bool reverse = parser.isSet("reverse");
	if(reverse)
	{
		SortCondition sc;
		sc.field = FILE_MTIME;
		sc.order = ASC;
		query.addSortCondition(sc);
	}

	try
	{

		QHash<QString, bool> seenMap;
		auto results = dbService->search(query);

		for(const SearchResult &result : results)
		{
			const QString &absPath = result.fileData.absPath;
			if(!seenMap.contains(absPath))
			{
				seenMap[absPath] = true;
				Logger::info() << absPath << Qt::endl;
			}
		}
	}
	catch(LooqsGeneralException &e)
	{
		Logger::error() << "Exception:" << e.message << Qt::endl;
		return 1;
	}

	return 0;
}
