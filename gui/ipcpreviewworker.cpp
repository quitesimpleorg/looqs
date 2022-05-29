#include <QtConcurrent>
#include "ipcpreviewworker.h"
#include "previewgeneratormapfunctor.h"
IPCPreviewWorker::IPCPreviewWorker()
{
	this->connect(&previewWorkerWatcher, &QFutureWatcher<QByteArray>::resultReadyAt, this,
				  [this](int index) { emit previewGenerated(previewWorkerWatcher.resultAt(index)); });
	connect(&previewWorkerWatcher, &QFutureWatcher<QByteArray>::finished, this, [this] { emit finished(); });
}

void IPCPreviewWorker::start(RenderConfig config, const QVector<RenderTarget> &targets, QLocalSocket *peer)
{
	stop();
	auto mapFunctor = PreviewGeneratorMapFunctor();
	mapFunctor.setRenderConfig(config);

	previewWorkerWatcher.setFuture(QtConcurrent::mapped(targets, mapFunctor));
}

void IPCPreviewWorker::stop()
{
	previewWorkerWatcher.cancel();
	previewWorkerWatcher.waitForFinished();
}
