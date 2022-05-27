#include <QMutexLocker>
#include <QPainter>
#include "previewgeneratorpdf.h"

static QMutex cacheMutex;

Poppler::Document *PreviewGeneratorPdf::document(QString path)
{
	if(documentcache.contains(path))
	{
		return documentcache.value(path);
	}
	Poppler::Document *result = Poppler::Document::load(path);
	if(result == nullptr)
	{
		// TODO: some kind of user feedback would be nice
		return nullptr;
	}
	result->setRenderHint(Poppler::Document::TextAntialiasing);
	QMutexLocker locker(&cacheMutex);
	documentcache.insert(path, result);
	locker.unlock();
	return result;
}

QSharedPointer<PreviewResult> PreviewGeneratorPdf::generate(RenderConfig config, QString documentPath,
															unsigned int page)
{
	PreviewResultPdf *result = new PreviewResultPdf(documentPath, page);
	Poppler::Document *doc = document(documentPath);
	if(doc == nullptr)
	{
		return QSharedPointer<PreviewResult>(result);
	}
	if(doc->isLocked())
	{
		return QSharedPointer<PreviewResult>(result);
	}
	int p = (int)page - 1;
	if(p < 0)
	{
		p = 0;
	}
	Poppler::Page *pdfPage = doc->page(p);
	QImage img = pdfPage->renderToImage(config.scaleX, config.scaleY);
	for(QString &word : config.wordsToHighlight)
	{
		QList<QRectF> rects = pdfPage->search(word, Poppler::Page::SearchFlag::IgnoreCase);
		for(QRectF &rect : rects)
		{
			QPainter painter(&img);
			painter.scale(config.scaleX / 72.0, config.scaleY / 72.0);
			painter.fillRect(rect, QColor(255, 255, 0, 64));
		}
	}
	result->previewImage = img;
	return QSharedPointer<PreviewResult>(result);
}
