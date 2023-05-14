#include "tagstripperprocessor.h"

TagStripperProcessor::TagStripperProcessor()
{
}

DocumentProcessResult TagStripperProcessor::process(const QByteArray &data) const
{
	auto result = DefaultTextProcessor::process(data);
	// TODO: does not work properly with <br> and does not deal with entities...
	Q_ASSERT(result.pages.size() > 0);
	result.pages[0].content.remove(QRegExp("<[^>]*>"));
	return result;
}
