#ifndef COMMANDLIST_H
#define COMMANDLIST_H
#include "command.h"
#include "../shared/sqlitesearch.h"

class CommandList : public Command
{
  public:
	using Command::Command;

	int handle(QStringList arguments) override;
};

#endif // COMMANDLIST_H
