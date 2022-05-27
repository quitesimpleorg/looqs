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

	QByteArray serialize() const;
	static QSharedPointer<PreviewResultPlainText> deserialize(QByteArray &ba);
};

#endif // PREVIEWRESULTPLAINTEXT_H
