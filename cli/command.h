#ifndef COMMAND_H
#define COMMAND_H
#include <QStringList>
#include <QThreadStorage>
#include <QVariant>
#include <QMutex>
#include <QWaitCondition>
#include "utils.h"
#include "sqlitedbservice.h"
class Command : public QObject
{
	Q_OBJECT
  signals:
	void finishedCmd(int retval);

  protected:
	SqliteDbService *dbService;
	QString dbConnectionString;
	QStringList arguments;

	bool autoFinish = true;

  public:
	Command(SqliteDbService &dbService)
	{
		this->dbService = &dbService;
	}
	void setArguments(QStringList arguments)
	{
		this->arguments = arguments;
	}
	virtual int handle(QStringList arguments) = 0;
	virtual ~Command(){};

  public slots:
	void execute();
};

#endif // COMMAND_H
