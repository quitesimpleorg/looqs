#include "previewresult.h"

PreviewResult::PreviewResult()
{
}

PreviewResult::~PreviewResult()
{
}

QWidget *PreviewResult::createPreviewWidget()
{
	return nullptr;
}

bool PreviewResult::hasPreview()
{
	return false;
}

PreviewResult::PreviewResult(QString documentPath, unsigned int page)
{
	this->documentPath = documentPath;
	this->page = page;
}

QString PreviewResult::getDocumentPath() const
{
	return this->documentPath;
}

unsigned int PreviewResult::getPage() const
{
	return this->page;
}
