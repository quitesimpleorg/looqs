#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include "sandboxedprocessor.h"
#include "pdfprocessor.h"
#include "defaulttextprocessor.h"
#include "tagstripperprocessor.h"
#include "nothingprocessor.h"
#include "odtprocessor.h"
#include "odsprocessor.h"
#include "../submodules/exile.h/exile.h"
#include "logger.h"

static DefaultTextProcessor *defaultTextProcessor = new DefaultTextProcessor();
static TagStripperProcessor *tagStripperProcessor = new TagStripperProcessor();
static NothingProcessor *nothingProcessor = new NothingProcessor();
static OdtProcessor *odtProcessor = new OdtProcessor();
static OdsProcessor *odsProcessor = new OdsProcessor();

static QMap<QString, Processor *> processors{
	{"pdf", new PdfProcessor()},	{"txt", defaultTextProcessor}, {"md", defaultTextProcessor},
	{"py", defaultTextProcessor},	{"xml", nothingProcessor},	   {"html", tagStripperProcessor},
	{"java", defaultTextProcessor}, {"js", defaultTextProcessor},  {"cpp", defaultTextProcessor},
	{"c", defaultTextProcessor},	{"sql", defaultTextProcessor}, {"odt", odtProcessor},
	{"ods", odsProcessor}};

void SandboxedProcessor::enableSandbox(QString readablePath)
{
	struct exile_policy *policy = exile_init_policy();
	if(policy == NULL)
	{
		qCritical() << "Could not init exile";
		exit(EXIT_FAILURE);
	}
	policy->namespace_options = EXILE_UNSHARE_NETWORK | EXILE_UNSHARE_USER;

	if(!readablePath.isEmpty())
	{
		std::string readablePathLocation = readablePath.toStdString();
		if(exile_append_path_policy(policy, EXILE_FS_ALLOW_ALL_READ, readablePathLocation.c_str()) != 0)
		{
			qCritical() << "Failed to add path policies";
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		policy->no_fs = 1;
	}
	int ret = exile_enable_policy(policy);
	if(ret != 0)
	{
		qDebug() << "Failed to establish sandbox: " << ret;
		exit(EXIT_FAILURE);
	}
	exile_free_policy(policy);
}

void SandboxedProcessor::printResults(const QVector<PageData> &pageData)
{
	QFile fsstdout;
	fsstdout.open(stdout, QIODevice::WriteOnly);
	QDataStream stream(&fsstdout);

	for(const PageData &data : pageData)
	{
		stream << data;
		// fsstdout.flush();
	}

	fsstdout.close();
}

int SandboxedProcessor::process()
{
	QFileInfo fileInfo(this->filePath);
	Processor *processor = processors.value(fileInfo.suffix(), nothingProcessor);

	if(processor == nothingProcessor)
	{
		/* Nothing to do */
		return NOTHING_PROCESSED;
	}

	QVector<PageData> pageData;
	QString absPath = fileInfo.absoluteFilePath();

	try
	{
		if(processor->PREFERED_DATA_SOURCE == FILEPATH)
		{
			/* Read access to FS needed... doh..*/
			enableSandbox(absPath);
			pageData = processor->process(absPath);
		}
		else
		{
			QByteArray data = Utils::readFile(absPath);
			enableSandbox();
			pageData = processor->process(data);
		}
	}
	catch(LooqsGeneralException &e)
	{
		Logger::error() << "Error while processing" << absPath << ":" << e.message << Qt::endl;
		return 3 /* PROCESSFAIL */;
	}

	printResults(pageData);
	return 0;
}