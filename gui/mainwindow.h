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
#include <QProgressDialog>
#include "../shared/looqsquery.h"
#include "../shared/indexsyncer.h"
#include "previewcoordinator.h"
#include "indexer.h"
namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

  private:
	DatabaseFactory *dbFactory;
	SqliteDbService *dbService;
	Ui::MainWindow *ui;

	PreviewCoordinator previewCoordinator;

	QThread syncerThread;
	Indexer *indexer;
	IndexSyncer *indexSyncer;
	QProgressDialog progressDialog;
	QFileIconProvider iconProvider;
	QSqlDatabase db;
	QFutureWatcher<QVector<SearchResult>> searchWatcher;
	LooqsQuery contentSearchQuery;
	QVector<QString> searchHistory;
	int currentSearchHistoryIndex = 0;
	QString currentSavedSearchText;
	bool previewDirty = false;
	int previewsPerPage = 20;

	void connectSignals();
	void makePreviews(int page);
	bool previewTabActive();
	bool indexerTabActive();
	void keyPressEvent(QKeyEvent *event) override;
	void handleSearchResults(const QVector<SearchResult> &results);
	void handleSearchError(QString error);
	void createSearchResultMenu(QMenu &menu, const QFileInfo &fileInfo);
	void openDocument(QString path, int num);
	void openFile(QString path);
	void initSettingsTabs();
	int currentSelectedScale();
	void processShortcut(int key);
	bool eventFilter(QObject *object, QEvent *event) override;

  private slots:
	void lineEditReturnPressed();
	void treeSearchItemActivated(QTreeWidgetItem *item, int i);
	void showSearchResultsContextMenu(const QPoint &point);
	void tabChanged();
	void previewReceived();
	void comboScaleChanged(int i);
	void spinPreviewPageValueChanged(int val);
	void startIndexing();
	void finishIndexing();
	void addPathToIndex();
	void startIndexSync();
	void saveSettings();
	void exportFailedPaths();

  signals:
	void startIpcPreviews(RenderConfig config, const QVector<RenderTarget> &targets);
	void stopIpcPreviews();
	void beginIndexSync();

  public:
	explicit MainWindow(QWidget *parent, QString socketPath);
	~MainWindow();
  signals:
	void beginSearch(const QString &query);
	void startPdfPreviewGeneration(QVector<SearchResult> paths, double scalefactor);

  protected:
	void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
