#include "previewresultpdf.h"

PreviewResultPdf::PreviewResultPdf(const PreviewResult &o)
{
	this->documentPath = o.getDocumentPath();
	this->page = o.getPage();
}

QWidget *PreviewResultPdf::createPreviewWidget()
{
	ClickLabel *label = new ClickLabel();
	label->setPixmap(QPixmap::fromImage(previewImage));
	label->setToolTip(getDocumentPath());
	return label;
}

bool PreviewResultPdf::hasPreview()
{
	bool result = !this->previewImage.isNull();
	return result;
}
