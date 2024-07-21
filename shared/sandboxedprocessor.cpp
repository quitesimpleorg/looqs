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
#include "common.h"
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
	{"ods", odsProcessor},			{"svg", nothingProcessor}};

void SandboxedProcessor::enableSandbox(QString readablePath)
{
	if(Common::noSandboxModeRequested())
	{
		qInfo() << "Sandbox is disabled!" << Qt::endl;
		return;
	}
	struct exile_policy *policy = exile_init_policy();
	if(policy == NULL)
	{
		qCritical() << "Could not init exile" << Qt::endl;
		exit(PROCESSFAIL);
	}
	policy->namespace_options = EXILE_UNSHARE_NETWORK | EXILE_UNSHARE_USER;

	std::string readablePathLocation;
	if(!readablePath.isEmpty())
	{
		policy->namespace_options |= EXILE_UNSHARE_MOUNT;
		policy->mount_path_policies_to_chroot = 1;
		readablePathLocation = readablePath.toStdString();
		if(exile_append_path_policies(policy, EXILE_FS_ALLOW_ALL_READ, readablePathLocation.c_str()) != 0)
		{
			qCritical() << "Failed to add path policies";
			exit(PROCESSFAIL);
		}
	}
	else
	{
		policy->no_fs = 1;
	}
	int ret = exile_enable_policy(policy);
	if(ret != 0)
	{
		qCritical() << "Failed to establish sandbox: " << ret;
		exit(PROCESSFAIL);
	}
	exile_free_policy(policy);
}

void SandboxedProcessor::printResults(const DocumentProcessResult &result)
{
	QFile fsstdout;
	fsstdout.open(stdout, QIODevice::WriteOnly);
	QDataStream stream(&fsstdout);
	stream << result;
	fsstdout.close();
}

SaveFileResult SandboxedProcessor::process()
{
	QFileInfo fileInfo(this->filePath);
	Processor *processor = processors.value(fileInfo.suffix(), nullptr);
	if(processor == nullptr)
	{
		/* TODO: Not sandboxed */
		if(Common::isTextFile(fileInfo))
		{
			processor = defaultTextProcessor;
		}
	}
	if(!fileInfo.isReadable())
	{
		return NOACCESS;
	}
	if(processor == nullptr || processor == nothingProcessor)
	{
		/* Nothing to do */
		return OK;
	}

	DocumentProcessResult processResult;
	QString absPath = fileInfo.absoluteFilePath();

	try
	{
		if(processor->PREFERED_DATA_SOURCE == FILEPATH)
		{
			/* Read access to FS needed... doh..*/
			enableSandbox(absPath);
			processResult = processor->process(absPath);
		}
		else
		{
			QByteArray data = Utils::readFile(absPath);
			enableSandbox();
			processResult = processor->process(data);
		}
	}
	catch(LooqsGeneralException &e)
	{
		Logger::error() << "SandboxedProcessor: Error while processing" << absPath << ":" << e.message << Qt::endl;
		return PROCESSFAIL;
	}

	printResults(processResult);
	return processResult.pages.isEmpty() ? OK_WASEMPTY : OK;
}
