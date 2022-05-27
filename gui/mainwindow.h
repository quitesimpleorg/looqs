#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTreeWidgetItem>
#include <QFileIconProvider>
#include <QKeyEvent>
#include <QFutureWatcher>
#include <QSqlDatabase>
#include <QLocalSocket>
#include "../shared/looqsquery.h"
#include "ipcpreviewclient.h"
#include "indexer.h"
namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	explicit MainWindow(QWidget *parent, QString socketPath);
	~MainWindow();
  signals:
	void beginSearch(const QString &query);
	void startPdfPreviewGeneration(QVector<SearchResult> paths, double scalefactor);

  private:
	DatabaseFactory *dbFactory;
	SqliteDbService *dbService;
	Ui::MainWindow *ui;
	IPCPreviewClient ipcPreviewClient;
	QThread ipcClientThread;

	Indexer *indexer;
	QFileIconProvider iconProvider;
	bool previewDirty;
	QSqlDatabase db;
	QFutureWatcher<QVector<SearchResult>> searchWatcher;
	void add(QString path, unsigned int page);
	QVector<SearchResult> previewableSearchResults;
	void connectSignals();
	void makePreviews(int page);
	bool previewTabActive();
	bool indexerTabActive();
	void keyPressEvent(QKeyEvent *event) override;
	unsigned int processedPdfPreviews;
	void handleSearchResults(const QVector<SearchResult> &results);
	void handleSearchError(QString error);
	LooqsQuery contentSearchQuery;
	int previewsPerPage;
	void createSearchResutlMenu(QMenu &menu, const QFileInfo &fileInfo);
	void ipcDocOpen(QString path, int num);
	void ipcFileOpen(QString path);
	unsigned int currentPreviewGeneration = 1;
  private slots:
	void lineEditReturnPressed();
	void treeSearchItemActivated(QTreeWidgetItem *item, int i);
	void showSearchResultsContextMenu(const QPoint &point);
	void tabChanged();
	void previewReceived(QSharedPointer<PreviewResult> preview, unsigned int previewGeneration);
	void comboScaleChanged(int i);
	void spinPreviewPageValueChanged(int val);
	void startIndexing();
	void finishIndexing();
	void addPathToIndex();

  signals:
	void startIpcPreviews(RenderConfig config, const QVector<RenderTarget> &targets);
	void stopIpcPreviews();
};

#endif // MAINWINDOW_H
