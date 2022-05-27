#ifndef IPCPREVIEWCLIENT_H
#define IPCPREVIEWCLIENT_H
#include <QObject>
#include <QLocalSocket>
#include "previewresult.h"
#include "renderconfig.h"
#include "rendertarget.h"

class IPCPreviewClient : public QObject
{
	Q_OBJECT
  private:
	unsigned int currentPreviewGeneration = 1;
	QLocalSocket *socket;
	QString socketPath;

	bool connect();
	QSharedPointer<PreviewResult> deserialize(QByteArray &array);

  public:
	IPCPreviewClient();
	~IPCPreviewClient()
	{
		delete socket;
	}
	void setSocketPath(QString socketPath);
  public slots:
	void start(RenderConfig config, const QVector<RenderTarget> &targets);
	void startGeneration(RenderConfig config, const QVector<RenderTarget> &targets);
	void stopGeneration();
  signals:
	void previewReceived(QSharedPointer<PreviewResult> previewResult, unsigned int currentPreviewGeneration);
	void finished();
};

#endif // IPCPREVIEWCLIENT_H
