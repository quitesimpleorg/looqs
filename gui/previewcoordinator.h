#ifndef PREVIEWCOORDINATOR_H
#define PREVIEWCOORDINATOR_H
#include <QVector>
#include <QObject>
#include <QThread>
#include "searchresult.h"
#include "previewresult.h"
#include "ipcpreviewclient.h"
#include "rendertarget.h"
class PreviewCoordinator : public QObject
{
	Q_OBJECT
  private:
	QThread ipcClientThread;
	IPCPreviewClient ipcPreviewClient;
	QString socketPath;

	QVector<QSharedPointer<PreviewResult>> previewResults;
	QVector<SearchResult> previewableSearchResults;

	unsigned int currentPreviewGeneration = 1;

	/* Quick lookup table for the order a preview should have */
	QHash<QString, int> previewOrder;

  public:
	PreviewCoordinator();

	void init(const QVector<SearchResult> &searchResults);

	int previewableCount() const;
	const QVector<SearchResult> &getPreviewableSearchResults() const;

	QSharedPointer<PreviewResult> resultAt(int index);

	void setSocketPath(QString socketPath);
  public slots:
	void startGeneration(RenderConfig config, const QVector<RenderTarget> &targets);
	void handleReceivedPreview(QSharedPointer<PreviewResult> preview, unsigned int previewGeneration);

  signals:
	void previewReady();
	void completedGeneration();
	void error(QString);
	void ipcStartGeneration(RenderConfig config, const QVector<RenderTarget> &targets);
};

#endif // PREVIEWCOORDINATOR_H
