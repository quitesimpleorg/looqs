#ifndef PDFPROCESSOR_H
#define PDFPROCESSOR_H
#include <poppler-qt5.h>
#include "processor.h"
class PdfProcessor : public Processor
{
  public:
	PdfProcessor();

  public:
	QVector<DocumentOutlineEntry> createOutline(const QVector<Poppler::OutlineItem> &outlineItems) const;
	DocumentProcessResult process(const QByteArray &data) const override;
};

#endif // PDFPROCESSOR_H
