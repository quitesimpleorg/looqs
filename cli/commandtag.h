#ifndef COMMANDTAG_H
#define COMMANDTAG_H
#include "command.h"

class CommandTag : public Command
{
  protected:
	bool ensureAbsolutePaths(const QVector<QString> &paths, QVector<QString> &absolutePaths);

  public:
	using Command::Command;

	int handle(QStringList arguments) override;
};

#endif // COMMANDTAG_H
