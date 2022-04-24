#ifndef PREVIEWRESULTPLAINTEXT_H
#define PREVIEWRESULTPLAINTEXT_H
#include "previewresult.h"

class PreviewResultPlainText : public PreviewResult
{
  private:
	QString text;

  public:
	using PreviewResult::PreviewResult;
	PreviewResultPlainText(const PreviewResult &o);

	QWidget *createPreviewWidget() override;
	bool hasPreview() override;

	void setText(QString text);
};

#endif // PREVIEWRESULTPLAINTEXT_H
