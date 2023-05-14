#ifndef SANDBOXEDPROCESSOR_H
#define SANDBOXEDPROCESSOR_H
#include <QString>
#include <QMimeDatabase>
#include "documentprocessresult.h"
#include "savefileresult.h"

class SandboxedProcessor
{
  private:
	QString filePath;
	QMimeDatabase mimeDatabase;

	void enableSandbox(QString readablePath = "");
	void printResults(const DocumentProcessResult &pageData);

  public:
	SandboxedProcessor(QString filepath)
	{
		this->filePath = filepath;
	}

	SaveFileResult process();
};

#endif // SANDBOXEDPROCESSOR_H
