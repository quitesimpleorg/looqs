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
#include "pdfworker.h"
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
	bool pdfDirty;
	QSqlDatabase db;
	QFutureWatcher<QVector<SearchResult>> searchWatcher;
	QFutureWatcher<PdfPreview> pdfWorkerWatcher;
	void add(QString path, unsigned int page);
	QVector<SearchResult> pdfSearchResults;
	void connectSignals();
	void makePdfPreview(int page);
	bool pdfTabActive();
	void keyPressEvent(QKeyEvent *event) override;
	unsigned int processedPdfPreviews;
	void handleSearchResults(const QVector<SearchResult> &results);
	void handleSearchError(QString error);
	LooqsQuery currentQuery;
	int pdfPreviewsPerPage;
	void createSearchResutlMenu(QMenu &menu, const QFileInfo &fileInfo);
	void ipcDocOpen(QString path, int num);
	void ipcFileOpen(QString path);

  private slots:
	void lineEditReturnPressed();
	void treeSearchItemActivated(QTreeWidgetItem *item, int i);
	void showSearchResultsContextMenu(const QPoint &point);
	void tabChanged();
	void pdfPreviewReceived(PdfPreview preview);
	void comboScaleChanged(int i);
	void spinPdfPreviewPageValueChanged(int val);
};

#endif // MAINWINDOW_H
