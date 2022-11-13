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

QByteArray PreviewResultPlainText::serialize() const
{
	QByteArray result;
	QDataStream stream{&result, QIODevice::WriteOnly};
	PreviewResultType type = PreviewResultType::PlainText;
	stream << type << this->documentPath << this->page << this->text;
	return result;
}

QSharedPointer<PreviewResultPlainText> PreviewResultPlainText::deserialize(QByteArray &ba)
{
	QSharedPointer<PreviewResultPlainText> result(new PreviewResultPlainText());

	PreviewResultType type;

	QDataStream stream{&ba, QIODevice::ReadOnly};
	stream >> type;
	if(type != PreviewResultType::PlainText)
	{
		throw std::runtime_error("Invalid byte array: Not a pdf preview");
	}
	stream >> result->documentPath >> result->page >> result->text;
	return result;
}
