#include "previewresultplaintext.h"

PreviewResultPlainText::PreviewResultPlainText(const PreviewResult &o)
{
	this->documentPath = o.getDocumentPath();
	this->page = o.getPage();
}

QWidget *PreviewResultPlainText::createPreviewWidget()
{

	ClickLabel *label = new ClickLabel();
	label->setText(this->text);
	label->setToolTip(getDocumentPath());
	label->setStyleSheet("border: 1px solid black");
	label->setMaximumWidth(768);
	label->setMaximumHeight(512);
	label->setTextFormat(Qt::RichText);
	return label;
}

bool PreviewResultPlainText::hasPreview()
{
	return !text.isEmpty();
}

void PreviewResultPlainText::setText(QString text)
{
	this->text = text;
}
