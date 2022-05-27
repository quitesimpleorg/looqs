#include <QtConcurrent>
#include "ipcpreviewworker.h"
#include "previewgeneratormapfunctor.h"
IPCPreviewWorker::IPCPreviewWorker()
{
}

void IPCPreviewWorker::start(RenderConfig config, const QVector<RenderTarget> &targets, QLocalSocket *peer)
{
	connect(&previewWorkerWatcher, &QFutureWatcher<QByteArray>::resultReadyAt, this,
			[peer, this](int index)
			{
				QDataStream stream{peer};
				stream << previewWorkerWatcher.resultAt(index);
				peer->flush();
			});
	connect(&previewWorkerWatcher, &QFutureWatcher<QByteArray>::finished, this,
			[peer]
			{
				/* TODO /

				/*peer->waitForBytesWritten();
				peer->disconnectFromServer();
				peer->deleteLater();*/
			});

	auto mapFunctor = new PreviewGeneratorMapFunctor();
	mapFunctor->setRenderConfig(config);

	previewWorkerWatcher.setFuture(QtConcurrent::mapped(targets, *mapFunctor));
}
