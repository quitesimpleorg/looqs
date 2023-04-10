#include <QCommandLineParser>
#include "commandtag.h"
#include "logger.h"
#include "tagmanager.h"

int CommandTag::handle(QStringList arguments)
{
	QCommandLineParser parser;
	parser.addPositionalArgument("add", "Adds a tag to a file",
								 "add [tag] [paths...]. Adds the tag to the specified paths");
	parser.addPositionalArgument("remove", "Removes a file associated to tag", "remove [tag] [file]");
	parser.addPositionalArgument("delete", "Deletes a tag", "delete [tag]");
	parser.addPositionalArgument("list", "Lists paths associated with a tag, or all tags", "list [tag]");
	parser.addHelpOption();

	parser.parse(arguments);

	QStringList args = parser.positionalArguments();
	if(args.length() == 0)
	{
		parser.showHelp(EXIT_FAILURE);
		return EXIT_FAILURE;
	}

	QString cmd = args[0];
	qDebug() << cmd;
	if(cmd == "add")
	{
		if(args.length() < 3)
		{
			Logger::error() << "Not enough arguments provided. 'add' requires a tag followed by at least one path"
							<< Qt::endl;
			return EXIT_FAILURE;
		}
		QString tag = args[1];
		auto paths = args.mid(2).toVector();
		for(int i = 0; i < paths.size(); i++)
		{
			QFileInfo info{paths[i]};
			if(!info.exists())
			{
				Logger::error() << "Can't add tag for file  " + info.absoluteFilePath() + " because it does not exist"
								<< Qt::endl;
				return EXIT_FAILURE;
			}
			QString absolutePath = info.absoluteFilePath();
			if(!this->dbService->fileExistsInDatabase(absolutePath))
			{
				Logger::error() << "Only files that have been indexed can be tagged. File not in index: " + absolutePath
								<< Qt::endl;
				return EXIT_FAILURE;
			}
			paths[i] = absolutePath;
		}

		TagManager tagManager{*this->dbService};
		bool result = tagManager.addPathsToTag(tag, paths);
		if(!result)
		{
			Logger::error() << "Failed to assign tags" << Qt::endl;
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	return 0;
}
