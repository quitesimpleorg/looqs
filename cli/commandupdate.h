#ifndef COMMANDUPDATE_H
#define COMMANDUPDATE_H
#include "command.h"
#include "filesaver.h"
class CommandUpdate : public Command
{
  protected:
	bool hasErrors = false;

  public:
	using Command::Command;
	int handle(QStringList arguments) override;
};

#endif // COMMANDUPDATE_H
