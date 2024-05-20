#include <QScopedPointer>
#include "pdfprocessor.h"
PdfProcessor::PdfProcessor()
{
}

QVector<DocumentOutlineEntry> PdfProcessor::createOutline(const QVector<Poppler::OutlineItem> &outlineItems) const
{
	QVector<DocumentOutlineEntry> result;
	for(const Poppler::OutlineItem &outlineItem : outlineItems)
	{
		DocumentOutlineEntry documentOutlineEntry;
		documentOutlineEntry.text = outlineItem.name();
		documentOutlineEntry.type = OUTLINE_DESTINATION_TYPE_PAGE;
		if(!outlineItem.destination().isNull())
		{
			documentOutlineEntry.destinationPage = outlineItem.destination()->pageNumber();
		}
		if(outlineItem.hasChildren())
		{
			documentOutlineEntry.children = createOutline(outlineItem.children());
		}
		result.append(documentOutlineEntry);
	}
	return result;
}

DocumentProcessResult PdfProcessor::process(const QByteArray &data) const
{
	DocumentProcessResult result;
	auto doc(Poppler::Document::loadFromData(data));
	if(!doc)
	{
		throw LooqsGeneralException("Failed to process pdf data");
	}
	if(doc->isLocked())
	{
		throw LooqsGeneralException("Doc is locked");
	}

	QRectF entirePage;

	auto pagecount = doc->numPages();
	QString entire;
	entire.reserve(data.size()); // TODO too much
	for(auto i = 0; i < pagecount; i++)
	{
		QString text = doc->page(i)->text(entirePage);
		result.pages.append({static_cast<unsigned int>(i + 1), text});
		/*TODO: hack, so we can fts search several words over the whole document, not just pages.
		 * this of course uses more space and should be solved differently.
		 */
		entire += text;
	}
	result.pages.append({0, entire});
	result.outlines = createOutline(doc->outline());
	return result;
}
