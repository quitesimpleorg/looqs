#ifndef PREVIEWRESULT_H
#define PREVIEWRESULT_H
#include "clicklabel.h"

enum PreviewResultType
{
	PDF = 1,
	PlainText
};

class PreviewResult
{
  protected:
	QString documentPath;
	unsigned int page;

  public:
	PreviewResult();
	PreviewResult(QString documentPath, unsigned int page);
	PreviewResult(const PreviewResult &o) = default;
	virtual ~PreviewResult();
	virtual QWidget *createPreviewWidget();
	virtual bool hasPreview();
	QString getDocumentPath() const;
	unsigned int getPage() const;
	virtual QByteArray serialize() const;
};

#endif // PREVIEWRESULT_H
