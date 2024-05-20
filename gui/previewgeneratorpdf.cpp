#include <QMutexLocker>
#include <QPainter>
#include <QRegularExpression>
#include "previewgeneratorpdf.h"

static QMutex cacheMutex;

Poppler::Document *PreviewGeneratorPdf::document(QString path)
{
	QMutexLocker locker(&cacheMutex);
	if(documentcache.contains(path))
	{
		return documentcache.value(path);
	}
	locker.unlock();
	auto result = Poppler::Document::load(path);
	if(!result)
	{
		qDebug() << "Failed to load document: " << path;
		// TODO: some kind of user feedback would be nice
		return nullptr;
	}
	result->setRenderHint(Poppler::Document::TextAntialiasing);
	result->setRenderHint(Poppler::Document::TextHinting);
	result->setRenderHint(Poppler::Document::TextSlightHinting);
	auto ptr = result.release();
	locker.relock();
	documentcache.insert(path, ptr);
	locker.unlock();
	return ptr;
}

QSharedPointer<PreviewResult> PreviewGeneratorPdf::generate(RenderConfig config, QString documentPath,
															unsigned int page)
{
	PreviewResultPdf *result = new PreviewResultPdf(documentPath, page);
	Poppler::Document *doc = document(documentPath);
	if(doc == nullptr)
	{
		qDebug() << "Failed to obtain document for: " << documentPath;
		return QSharedPointer<PreviewResult>(result);
	}
	if(doc->isLocked())
	{
		qDebug() << "Failed to open document as its locked: " << documentPath;
		return QSharedPointer<PreviewResult>(result);
	}
	int p = (int)page - 1;
	if(p < 0)
	{
		p = 0;
	}
	auto pdfPage = doc->page(p);
	if(!pdfPage)
	{
		qDebug() << "Failed to open page " << p << " for document" << documentPath;
		return QSharedPointer<PreviewResult>(result);
	}
	QImage img = pdfPage->renderToImage(config.scaleX, config.scaleY);
	for(QString &word : config.wordsToHighlight)
	{
		QList<QRectF> rects =
			pdfPage->search(word, Poppler::Page::SearchFlag::IgnoreCase | Poppler::Page::SearchFlag::WholeWords);
		if(rects.empty())
		{
			rects = pdfPage->search(word, Poppler::Page::SearchFlag::IgnoreCase);
		}
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
