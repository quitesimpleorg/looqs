#ifndef PREVIEWRESULTPDF_H
#define PREVIEWRESULTPDF_H
#include <QImage>
#include "previewresult.h"

class PreviewResultPdf : public PreviewResult
{
  public:
	using PreviewResult::PreviewResult;
	PreviewResultPdf(const PreviewResult &o);
	QImage previewImage;

	QWidget *createPreviewWidget() override;
	bool hasPreview() override;
};

#endif // PREVIEWRESULTPDF_H
