#ifndef DEFAULTTEXTPROCESSOR_H
#define DEFAULTTEXTPROCESSOR_H

#include "processor.h"
#include "encodingdetector.h"
class DefaultTextProcessor : public Processor
{
  protected:
	EncodingDetector encodingDetector;

  public:
	DefaultTextProcessor();
	QString processText(const QByteArray &data) const;
	DocumentProcessResult process(const QByteArray &data) const override;
};

#endif // DEFAULTTEXTPROCESSOR_H
