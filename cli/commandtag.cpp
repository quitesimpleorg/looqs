#include <QCommandLineParser>
#include "commandtag.h"
#include "logger.h"
#include "tagmanager.h"

bool CommandTag::ensureAbsolutePaths(const QVector<QString> &paths, QVector<QString> &absolutePaths)
{
	for(const QString &path : paths)
	{
		QFileInfo info{path};
		if(!info.exists())
		{
			Logger::error() << "Can't add tag for file  " + info.absoluteFilePath() + " because it does not exist"
							<< Qt::endl;
			return false;
		}
		QString absolutePath = info.absoluteFilePath();
		if(!this->dbService->fileExistsInDatabase(absolutePath))
		{
			Logger::error() << "Only files that have been indexed can be tagged. File not in index: " + absolutePath
							<< Qt::endl;
			return false;
		}
		absolutePaths.append(absolutePath);
	}
	return true;
}

int CommandTag::handle(QStringList arguments)
{
	QCommandLineParser parser;
	parser.addPositionalArgument("add", "Adds a tag to a file",
								 "add [tag] [paths...]. Adds the tag to the specified paths");
	parser.addPositionalArgument("remove", "Removes a path associated to a tag", "remove [tag] [path]");
	parser.addPositionalArgument("delete", "Deletes a tag", "delete [tag]");
	parser.addPositionalArgument("list", "Lists paths associated with a tag, or all tags", "list [tag]");
	parser.addPositionalArgument("show", "Lists tags associated with a path", "show [path]");

	parser.addHelpOption();

	parser.parse(arguments);

	QStringList args = parser.positionalArguments();
	if(args.length() == 0)
	{
		parser.showHelp(EXIT_FAILURE);
		return EXIT_FAILURE;
	}
	TagManager tagManager{*this->dbService};
	QString cmd = args[0];
	if(cmd == "add")
	{
		if(args.length() < 3)
		{
			Logger::error() << "Not enough arguments provided. 'add' requires a tag followed by at least one path"
							<< Qt::endl;
			return EXIT_FAILURE;
		}
		QString tag = args[1];
		QVector<QString> paths = args.mid(2).toVector();

		QVector<QString> absolutePaths;
		if(!ensureAbsolutePaths(paths, absolutePaths))
		{
			return EXIT_FAILURE;
		}
		bool result = tagManager.addPathsToTag(tag, absolutePaths);
		if(!result)
		{
			Logger::error() << "Failed to assign tags" << Qt::endl;
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
	if(cmd == "list")
	{

		QString tag;
		if(args.length() >= 2)
		{
			tag = args[1];
		}
		QVector<QString> entries;
		if(tag.isEmpty())
		{
			entries = tagManager.getTags();
		}
		else
		{
			entries = tagManager.getPaths(tag);
		}
		for(const QString &entry : entries)
		{
			Logger::info() << entry << Qt::endl;
		}
	}
	if(cmd == "remove")
	{
		if(args.length() < 3)
		{
			Logger::error() << "Not enough arguments provided. 'remove' requires a tag followed by at least one path"
							<< Qt::endl;
			return EXIT_FAILURE;
		}
		QString tag = args[1];
		QVector<QString> paths = args.mid(2).toVector();

		QVector<QString> absolutePaths;
		if(!ensureAbsolutePaths(paths, absolutePaths))
		{
			return EXIT_FAILURE;
		}

		if(!tagManager.removePathsForTag(tag, absolutePaths))
		{
			Logger::error() << "Failed to remove path assignments" << Qt::endl;
			return EXIT_FAILURE;
		}
	}
	if(cmd == "delete")
	{
		if(args.length() != 2)
		{
			Logger::error() << "The 'delete' command requires the tag to delete" << Qt::endl;
			return EXIT_FAILURE;
		}
		if(!tagManager.deleteTag(args[1]))
		{
			Logger::error() << "Failed to delete tag" << Qt::endl;
			return EXIT_FAILURE;
		}
	}
	if(cmd == "show")
	{
		if(args.length() != 2)
		{
			Logger::error() << "The 'show' command requires a path to show the assigned tags" << Qt::endl;
			return EXIT_FAILURE;
		}
		QString path = args[1];
		QVector<QString> absolutePaths;
		if(!ensureAbsolutePaths({path}, absolutePaths))
		{
			return EXIT_FAILURE;
		}
		QVector<QString> tags = tagManager.getTags(absolutePaths.at(0));
		for(const QString &entry : tags)
		{
			Logger::info() << entry << Qt::endl;
		}
	}
	return EXIT_SUCCESS;
}
