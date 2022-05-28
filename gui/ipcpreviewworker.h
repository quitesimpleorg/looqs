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

  public:
	IPCPreviewWorker();
	void start(RenderConfig config, const QVector<RenderTarget> &targets, QLocalSocket *peer);
	void stop();
  signals:
	void previewGenerated(QByteArray);
	void finished();
};

#endif // IPCPREVIEWWORKER_H
