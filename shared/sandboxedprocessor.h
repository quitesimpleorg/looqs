#ifndef SANDBOXEDPROCESSOR_H
#define SANDBOXEDPROCESSOR_H
#include <QString>
#include <QMimeDatabase>
#include "pagedata.h"

class SandboxedProcessor
{
  private:
	QString filePath;
	QMimeDatabase mimeDatabase;

	void enableSandbox(QString readablePath = "");
	void printResults(const QVector<PageData> &pageData);

  public:
	SandboxedProcessor(QString filepath)
	{
		this->filePath = filepath;
	}

	int process();
};

#endif // SANDBOXEDPROCESSOR_H
