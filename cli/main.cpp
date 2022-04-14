#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDataStream>
#include <QDebug>
#include <QProcessEnvironment>
#include <QMap>
#include <QDebug>
#include <QSettings>
#include <functional>
#include <QTimer>

#include <exception>
#include "encodingdetector.h"
#include "pdfprocessor.h"
#include "defaulttextprocessor.h"
#include "utils.h"
#include "command.h"
#include "commandadd.h"
#include "commanddelete.h"
#include "commandupdate.h"
#include "commandsearch.h"
#include "databasefactory.h"
#include "logger.h"
#include "sandboxedprocessor.h"
#include "../shared/common.h"
#include "../shared/filescanworker.h"

void printUsage(QString argv0)
{
	qInfo() << "Usage:" << argv0 << "command";
}

Command *commandFromName(QString name, SqliteDbService &dbService)
{
	if(name == "add")
	{
		return new CommandAdd(dbService);
	}
	if(name == "delete")
	{
		return new CommandDelete(dbService);
	}
	if(name == "update")
	{
		return new CommandUpdate(dbService);
	}
	if(name == "search")
	{
		return new CommandSearch(dbService);
	}

	return nullptr;
}

int main(int argc, char *argv[])
{
	Common::setupAppInfo();
	QCoreApplication app(argc, argv);
	QStringList args = app.arguments();
	QString argv0 = args.takeFirst();

	if(args.length() < 1)
	{
		printUsage(argv0);
		exit(1);
	}

	try
	{
		Common::ensureConfigured();
	}
	catch(LooqsGeneralException &e)
	{
		Logger::error() << "Error: " << e.message;
		return 1;
	}
	qRegisterMetaType<PageData>();
	qRegisterMetaType<FileScanResult>("FileScanResult");

	QString connectionString = Common::databasePath();
	DatabaseFactory dbFactory(connectionString);
	SqliteDbService dbService(dbFactory);
	QString commandName = args.first();
	if(commandName == "process")
	{
		if(args.length() < 1)
		{
			qDebug() << "Filename is required";
			return 1;
		}

		QString file = args.at(1);
		SandboxedProcessor processor(file);
		return processor.process();
	}
	Command *cmd = commandFromName(commandName, dbService);
	if(cmd != nullptr)
	{
		try
		{
			QObject::connect(cmd, &Command::finishedCmd, [](int retval) { QCoreApplication::exit(retval); });
			cmd->setArguments(args);
			QTimer::singleShot(0, cmd, &Command::execute);
		}
		catch(const LooqsGeneralException &e)
		{
			Logger::error() << "Exception caught, message: " << e.message << Qt::endl;
		}
	}
	else
	{
		Logger::error() << "Unknown command " << commandName << Qt::endl;
	}

	return app.exec();
}
