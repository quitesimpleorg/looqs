#include <QApplication>
#include <QScreen>
#include <QScopedPointer>
#include <QMutexLocker>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentMap>
#include <atomic>
#include "previewworker.h"

PreviewWorker::PreviewWorker()
{
}

QFuture<QSharedPointer<PreviewResult>> PreviewWorker::generatePreviews(const QVector<SearchResult> paths,
																	   QVector<QString> wordsToHighlight,
																	   double scalefactor)
{
	QVector<QSharedPointer<PreviewResult>> previews;

	for(const SearchResult &sr : paths)
	{
		for(unsigned int page : sr.pages)
		{
			QSharedPointer<PreviewResult> ptr =
				QSharedPointer<PreviewResult>(new PreviewResult{sr.fileData.absPath, page});
			previews.append(ptr);
		}
	}

	RenderConfig renderConfig;
	renderConfig.scaleX = QGuiApplication::primaryScreen()->physicalDotsPerInchX() * scalefactor;
	renderConfig.scaleY = QGuiApplication::primaryScreen()->physicalDotsPerInchY() * scalefactor;
	renderConfig.wordsToHighlight = wordsToHighlight;

	auto mapFunctor = new PreviewGeneratorMapFunctor();
	mapFunctor->setRenderConfig(renderConfig);

	return QtConcurrent::mapped(previews, *mapFunctor);
}
