#include <QtConcurrent>
#include "ipcpreviewworker.h"
#include "previewgeneratormapfunctor.h"
IPCPreviewWorker::IPCPreviewWorker(QLocalSocket *peer)
{
	this->peer = peer;
	this->connect(&previewWorkerWatcher, &QFutureWatcher<QByteArray>::resultReadyAt, this,
				  [this](int index)
				  {
					  if(this->peer != nullptr)
					  {
						  QDataStream stream{this->peer};
						  stream << previewWorkerWatcher.resultAt(index);
						  this->peer->flush();
					  }
				  });
	connect(&previewWorkerWatcher, &QFutureWatcher<QByteArray>::finished, this, &IPCPreviewWorker::shutdownSocket);
	connect(this->peer, &QLocalSocket::disconnected, this, &IPCPreviewWorker::shutdownSocket);
}

void IPCPreviewWorker::shutdownSocket()
{
	if(cleaned)
	{
		return;
	}
	cleaned = true;
	if(this->peer != nullptr)
	{
		if(this->peer->state() == QLocalSocket::ConnectedState)
		{
			this->peer->flush();
			this->peer->waitForBytesWritten();
			this->peer->disconnectFromServer();
			if(this->peer->state() != QLocalSocket::UnconnectedState)
			{
				this->peer->waitForDisconnected();
			}
		}
		delete this->peer;
		this->peer = nullptr;
	}
	emit finished();
}

void IPCPreviewWorker::start(RenderConfig config, const QVector<RenderTarget> &targets)
{
	auto mapFunctor = PreviewGeneratorMapFunctor();
	mapFunctor.setRenderConfig(config);

	previewWorkerWatcher.setFuture(QtConcurrent::mapped(targets, mapFunctor));
}

void IPCPreviewWorker::stop()
{
	previewWorkerWatcher.cancel();
	previewWorkerWatcher.waitForFinished();
}
