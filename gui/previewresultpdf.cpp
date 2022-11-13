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

QByteArray PreviewResultPdf::serialize() const
{
	QByteArray result;
	QDataStream stream{&result, QIODevice::WriteOnly};
	PreviewResultType type = PreviewResultType::PDF;
	stream << type << this->documentPath << this->page << this->previewImage;
	return result;
}

QSharedPointer<PreviewResultPdf> PreviewResultPdf::deserialize(QByteArray &ba)
{
	QSharedPointer<PreviewResultPdf> result(new PreviewResultPdf());
	PreviewResultType type;

	QDataStream stream{&ba, QIODevice::ReadOnly};
	stream >> type;
	if(type != PreviewResultType::PDF)
	{
		throw std::runtime_error("Invalid byte array: Not a pdf preview");
	}
	stream >> result->documentPath >> result->page >> result->previewImage;
	return result;
}
