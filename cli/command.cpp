#include <QFile>
#include <QThread>
#include <QDebug>
#include "command.h"

void Command::execute()
{
	int result = handle(arguments);
	if(autoFinish)
	{
		emit finishedCmd(result);
	}
}
