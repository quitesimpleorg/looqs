#ifndef XMLSTRIPPERPROCESSOR_H
#define XMLSTRIPPERPROCESSOR_H
#include "defaulttextprocessor.h"

class TagStripperProcessor : public DefaultTextProcessor
{
  public:
	TagStripperProcessor();

  public:
	DocumentProcessResult process(const QByteArray &data) const override;
};

#endif // XMLSTRIPPERPROCESSOR_H
