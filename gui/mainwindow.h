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
#include "previewworker.h"
#include "../shared/looqsquery.h"
#include "ipcclient.h"
namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	explicit MainWindow(QWidget *parent, IPCClient &client);
	~MainWindow();
  signals:
	void beginSearch(const QString &query);
	void startPdfPreviewGeneration(QVector<SearchResult> paths, double scalefactor);

  private:
	Ui::MainWindow *ui;
	IPCClient *ipcClient;
	QFileIconProvider iconProvider;
	bool previewDirty;
	QSqlDatabase db;
	QFutureWatcher<QVector<SearchResult>> searchWatcher;
	QFutureWatcher<QSharedPointer<PreviewResult>> previewWorkerWatcher;
	void add(QString path, unsigned int page);
	QVector<SearchResult> previewableSearchResults;
	void connectSignals();
	void makePreviews(int page);
	bool previewTabActive();
	void keyPressEvent(QKeyEvent *event) override;
	unsigned int processedPdfPreviews;
	void handleSearchResults(const QVector<SearchResult> &results);
	void handleSearchError(QString error);
	LooqsQuery contentSearchQuery;
	int previewsPerPage;
	void createSearchResutlMenu(QMenu &menu, const QFileInfo &fileInfo);
	void ipcDocOpen(QString path, int num);
	void ipcFileOpen(QString path);

  private slots:
	void lineEditReturnPressed();
	void treeSearchItemActivated(QTreeWidgetItem *item, int i);
	void showSearchResultsContextMenu(const QPoint &point);
	void tabChanged();
	void previewReceived(QSharedPointer<PreviewResult> preview);
	void comboScaleChanged(int i);
	void spinPreviewPageValueChanged(int val);
};

#endif // MAINWINDOW_H
