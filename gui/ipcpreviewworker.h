#ifndef IPCPREVIEWWORKER_H
#define IPCPREVIEWWORKER_H
#include <QLocalSocket>
#include <QFutureWatcher>
#include "renderconfig.h"
#include "rendertarget.h"
#include "previewgenerator.h"

class IPCPreviewWorker : public QObject
{
	Q_OBJECT
  private:
	QFutureWatcher<QByteArray> previewWorkerWatcher;
	QLocalSocket *peer;
	bool cleaned = false;

  public:
	IPCPreviewWorker(QLocalSocket *peer);
	void start(RenderConfig config, const QVector<RenderTarget> &targets);
	void stop();
  private slots:
	void shutdownSocket();

  signals:
	void finished();
};

#endif // IPCPREVIEWWORKER_H
