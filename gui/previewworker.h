#ifndef PREVIEWWORKER_H
#define PREVIEWWORKER_H
#include <QObject>
#include <QImage>
#include <QHash>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QMutex>
#include <QFuture>
#include "previewresultpdf.h"
#include "searchresult.h"
#include "previewgenerator.h"
#include "previewworker.h"
#include "previewgeneratorpdf.h"
#include "previewgeneratormapfunctor.h"

class PreviewWorker : public QObject
{
	Q_OBJECT
  public:
	PreviewWorker();
	QSharedPointer<PreviewGenerator> createGenerator(QString path);

	QFuture<QSharedPointer<PreviewResult>> generatePreviews(const QVector<SearchResult> paths,
															QVector<QString> wordsToHighlight, double scalefactor);
};

#endif // PREVIEWWORKER_H
