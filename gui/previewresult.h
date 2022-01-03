#ifndef PREVIEWRESULT_H
#define PREVIEWRESULT_H
#include "clicklabel.h"

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
};

#endif // PREVIEWRESULT_H
