#ifndef COMMANDADD_H
#define COMMANDADD_H
#include <QMutex>
#include "command.h"
#include "filesaver.h"
#include "indexer.h"

class CommandAdd : public Command
{
  private:
	SaveFileResult addFile(QString path);
	Indexer *indexer;
	bool keepGoing = true;

  protected:
	IndexResult currentResult;

  public:
	using Command::Command;

	int handle(QStringList arguments) override;
  private slots:
	void indexerFinished();
};

#endif // COMMANDADD_H
