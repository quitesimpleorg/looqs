#include <poppler-qt5.h>
#include <QLabel>
#include <QtDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QtGlobal>
#include <QSettings>
#include <QDateTime>
#include <QProcess>
#include <QComboBox>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QFileDialog>
#include <QScreen>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clicklabel.h"
#include "../shared/sqlitesearch.h"
#include "../shared/looqsgeneralexception.h"
#include "../shared/common.h"
#include "ipcpreviewclient.h"
#include "previewgenerator.h"

MainWindow::MainWindow(QWidget *parent, QString socketPath) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	setWindowTitle(QCoreApplication::applicationName());
	this->ipcPreviewClient.moveToThread(&this->ipcClientThread);
	this->ipcPreviewClient.setSocketPath(socketPath);

	connect(&ipcPreviewClient, &IPCPreviewClient::previewReceived, this, &MainWindow::previewReceived,
			Qt::QueuedConnection);
	connect(&ipcPreviewClient, &IPCPreviewClient::finished, this,
			[&] { this->ui->previewProcessBar->setValue(this->ui->previewProcessBar->maximum()); });

	connect(this, &MainWindow::startIpcPreviews, &ipcPreviewClient, &IPCPreviewClient::startGeneration,
			Qt::QueuedConnection);
	connect(this, &MainWindow::stopIpcPreviews, &ipcPreviewClient, &IPCPreviewClient::stopGeneration,
			Qt::QueuedConnection);
	this->ipcClientThread.start();
	QSettings settings;

	this->dbFactory = new DatabaseFactory(Common::databasePath());

	db = this->dbFactory->forCurrentThread();
	this->dbService = new SqliteDbService(*this->dbFactory);

	indexer = new Indexer(*(this->dbService));
	indexer->setParent(this);
	connectSignals();
	ui->treeResultsList->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	ui->tabWidget->setCurrentIndex(0);
	ui->statusBar->addWidget(ui->lblSearchResults);
	ui->statusBar->addWidget(ui->previewProcessBar);
	ui->previewProcessBar->hide();
	ui->comboScale->setCurrentText(settings.value("currentScale").toString());
	previewsPerPage = settings.value("previewsPerPage", 20).toInt();
	ui->spinPreviewPage->setMinimum(1);

	QStringList indexPaths = settings.value("indexPaths").toStringList();
	ui->lstPaths->addItems(indexPaths);
}

void MainWindow::addPathToIndex()
{
	QString path = this->ui->txtPathScanAdd->text();
	QFileInfo fileInfo{path};
	if(!fileInfo.exists(path))
	{
		QMessageBox::critical(this, "Invalid path", "Path does not seem to exist");
		return;
	}
	if(!fileInfo.isReadable())
	{
		QMessageBox::critical(this, "Invalid path", "Path cannot be read");
		return;
	}
	this->ui->lstPaths->addItem(path);
	this->ui->txtPathScanAdd->clear();
}
void MainWindow::connectSignals()
{
	connect(ui->txtSearch, &QLineEdit::returnPressed, this, &MainWindow::lineEditReturnPressed);
	connect(&searchWatcher, &QFutureWatcher<SearchResult>::finished, this,
			[&]
			{
				try
				{
					this->ui->txtSearch->setEnabled(true);
					auto results = searchWatcher.future().result();
					handleSearchResults(results);
				}
				catch(LooqsGeneralException &e)
				{
					handleSearchError(e.message);
				}
			});
	connect(ui->treeResultsList, &QTreeWidget::itemActivated, this, &MainWindow::treeSearchItemActivated);
	connect(ui->treeResultsList, &QTreeWidget::customContextMenuRequested, this,
			&MainWindow::showSearchResultsContextMenu);
	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);
	connect(ui->comboScale, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::comboScaleChanged);
	connect(ui->spinPreviewPage, qOverload<int>(&QSpinBox::valueChanged), this,
			&MainWindow::spinPreviewPageValueChanged);

	connect(ui->btnAddPath, &QPushButton::clicked, this, &MainWindow::addPathToIndex);
	connect(ui->txtPathScanAdd, &QLineEdit::returnPressed, this, &MainWindow::addPathToIndex);
	connect(ui->btnStartIndexing, &QPushButton::clicked, this, &MainWindow::startIndexing);

	connect(this->indexer, &Indexer::pathsCountChanged, this,
			[&](int number)
			{
				ui->lblSearchResults->setText("Found paths: " + QString::number(number));
				ui->lblPathsFoundValue->setText(QString::number(number));
				ui->previewProcessBar->setMaximum(number);
			});
	connect(this->indexer, &Indexer::indexProgress, this,

			[&](int number, unsigned int added, unsigned int skipped, unsigned int failed, unsigned int totalCount)
			{
				ui->lblSearchResults->setText("Processed " + QString::number(number) + " files");
				ui->previewProcessBar->setValue(number);
				ui->previewProcessBar->setMaximum(totalCount);
				ui->lblAddedValue->setText(QString::number(added));
				ui->lblSkippedValue->setText(QString::number(skipped));
				ui->lblFailedValue->setText(QString::number(failed));
			});

	connect(this->indexer, &Indexer::finished, this, &MainWindow::finishIndexing);

	connect(ui->lstPaths->selectionModel(), &QItemSelectionModel::selectionChanged, this,
			[&](const QItemSelection &selected, const QItemSelection &deselected)
			{ ui->btnDeletePath->setEnabled(this->ui->lstPaths->selectedItems().count() > 0); });

	connect(ui->btnDeletePath, &QPushButton::clicked, this, [&] { qDeleteAll(ui->lstPaths->selectedItems()); });
	connect(ui->btnChoosePath, &QPushButton::clicked, this,
			[&]
			{
				QFileDialog dialog(nullptr);
				dialog.setFileMode(QFileDialog::Directory);
				dialog.setOptions(QFileDialog::ShowDirsOnly);
				if(dialog.exec())
				{
					auto paths = dialog.selectedFiles();
					if(paths.size() == 1)
					{
						ui->lstPaths->addItem(paths[0]);
					}
				}
			});
}

void MainWindow::spinPreviewPageValueChanged(int val)
{
	makePreviews(val);
}

void MainWindow::startIndexing()
{
	if(this->indexer->isRunning())
	{
		ui->btnStartIndexing->setEnabled(false);
		ui->btnStartIndexing->setText("Start indexing");
		this->indexer->requestCancellation();
		return;
	}

	ui->previewsTab->setEnabled(false);
	ui->resultsTab->setEnabled(false);
	ui->txtPathScanAdd->setEnabled(false);
	ui->txtSearch->setEnabled(false);
	ui->previewProcessBar->setValue(0);
	ui->previewProcessBar->setVisible(true);

	QVector<QString> paths;
	QStringList pathSettingsValue;
	for(int i = 0; i < ui->lstPaths->count(); i++)
	{
		QString path = ui->lstPaths->item(i)->text();
		paths.append(path);
		pathSettingsValue.append(path);
	}
	this->indexer->setTargetPaths(paths);
	this->indexer->beginIndexing();
	QSettings settings;
	settings.setValue("indexPaths", pathSettingsValue);
	ui->btnStartIndexing->setText("Stop indexing");
}

void MainWindow::finishIndexing()
{
	IndexResult result = this->indexer->getResult();

	ui->lblSearchResults->setText("Indexing finished");
	ui->previewProcessBar->setValue(ui->previewProcessBar->maximum());
	ui->lblFailedValue->setText(QString::number(result.erroredPaths));
	ui->lblSkippedValue->setText(QString::number(result.skippedPaths));
	ui->lblAddedValue->setText(QString::number(result.addedPaths));
	ui->btnStartIndexing->setEnabled(true);
	ui->btnStartIndexing->setText("Start indexing");
	ui->previewsTab->setEnabled(true);
	ui->resultsTab->setEnabled(true);
	ui->txtPathScanAdd->setEnabled(true);
	ui->txtSearch->setEnabled(true);
}

void MainWindow::comboScaleChanged(int i)
{
	QSettings scaleSetting;
	scaleSetting.setValue("currentScale", ui->comboScale->currentText());
	makePreviews(ui->spinPreviewPage->value());
}

bool MainWindow::previewTabActive()
{
	return ui->tabWidget->currentIndex() == 1;
}

bool MainWindow::indexerTabActive()
{
	return ui->tabWidget->currentIndex() == 2;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	bool quit =
		((event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Q) || event->key() == Qt::Key_Escape);
	if(quit)
	{
		qApp->quit();
	}

	if(event->modifiers() & Qt::ControlModifier)
	{

		if(event->key() == Qt::Key_L)
		{
			ui->txtSearch->setFocus();
			ui->txtSearch->selectAll();
		}
	}
	QWidget::keyPressEvent(event);
}

void MainWindow::tabChanged()
{
	if(ui->tabWidget->currentIndex() == 0)
	{
		ui->previewProcessBar->hide();
	}
	else
	{
		if(ui->previewProcessBar->value() > 0)
		{
			ui->previewProcessBar->show();
		}
	}
	if(previewTabActive())
	{
		if(previewDirty)
		{
			makePreviews(ui->spinPreviewPage->value());
		}
	}
}

void MainWindow::previewReceived(QSharedPointer<PreviewResult> preview, unsigned int previewGeneration)
{
	if(previewGeneration < this->currentPreviewGeneration)
	{
		return;
	}
	this->ui->previewProcessBar->setValue(this->ui->previewProcessBar->value() + 1);
	if(!preview.isNull() && preview->hasPreview())
	{
		QString docPath = preview->getDocumentPath();
		auto previewPage = preview->getPage();

		ClickLabel *label = dynamic_cast<ClickLabel *>(preview->createPreviewWidget());
		ui->scrollAreaWidgetContents->layout()->addWidget(label);
		connect(label, &ClickLabel::leftClick, [this, docPath, previewPage]() { ipcDocOpen(docPath, previewPage); });
		connect(label, &ClickLabel::rightClick,
				[this, docPath, previewPage]()
				{
					QFileInfo fileInfo{docPath};
					QMenu menu("labeRightClick", this);
					createSearchResutlMenu(menu, fileInfo);
					menu.addAction("Copy page number", [previewPage]
								   { QGuiApplication::clipboard()->setText(QString::number(previewPage)); });
					menu.exec(QCursor::pos());
				});
	}
}

void MainWindow::lineEditReturnPressed()
{
	QString q = ui->txtSearch->text();
	if(!LooqsQuery::checkParanthesis(q))
	{
		ui->lblSearchResults->setText("Invalid paranthesis");
		return;
	}
	// TODO: validate q;
	ui->treeResultsList->clear();
	ui->lblSearchResults->setText("Searching...");
	this->ui->txtSearch->setEnabled(false);
	QFuture<QVector<SearchResult>> searchFuture = QtConcurrent::run(
		[&, q]()
		{
			SqliteSearch searcher(db);
			QVector<SearchResult> results;
			this->contentSearchQuery = LooqsQuery::build(q, TokenType::FILTER_CONTENT_CONTAINS, true);

			/* We can have a path search in contentsearch too (if given explicitly), so no need to do it twice.
				Make sure path results are listed first. */
			bool addContentSearch = this->contentSearchQuery.hasContentSearch();
			bool addPathSearch = !this->contentSearchQuery.hasPathSearch() || !addContentSearch;
			if(addPathSearch)
			{
				LooqsQuery filesQuery = LooqsQuery::build(q, TokenType::FILTER_PATH_CONTAINS, false);
				results.append(searcher.search(filesQuery));
			}
			if(addContentSearch)
			{
				results.append(searcher.search(this->contentSearchQuery));
			}
			return results;
		});
	searchWatcher.setFuture(searchFuture);
}

void MainWindow::handleSearchResults(const QVector<SearchResult> &results)
{
	this->previewableSearchResults.clear();
	ui->treeResultsList->clear();

	bool hasDeleted = false;
	for(const SearchResult &result : results)
	{
		QFileInfo pathInfo(result.fileData.absPath);

		QString fileName = pathInfo.fileName();
		QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeResultsList);

		QDateTime dt = QDateTime::fromSecsSinceEpoch(result.fileData.mtime);
		item->setIcon(0, iconProvider.icon(pathInfo));
		item->setText(0, fileName);
		item->setText(1, result.fileData.absPath);
		item->setText(2, dt.toString(Qt::ISODate));
		item->setText(3, this->locale().formattedDataSize(result.fileData.size));
		bool exists = pathInfo.exists();
		if(exists)
		{
			if(PreviewGenerator::get(pathInfo) != nullptr)
			{
				this->previewableSearchResults.append(result);
			}
		}
		else
		{
			hasDeleted = true;
		}
	}
	ui->treeResultsList->resizeColumnToContents(0);
	ui->treeResultsList->resizeColumnToContents(1);
	previewDirty = !this->previewableSearchResults.empty();

	int numpages = ceil(static_cast<double>(this->previewableSearchResults.size()) / previewsPerPage);
	ui->spinPreviewPage->setMinimum(1);
	ui->spinPreviewPage->setMaximum(numpages);
	ui->spinPreviewPage->setValue(1);
	if(previewTabActive() && previewDirty)
	{
		makePreviews(1);
	}

	QString statusText = "Results: " + QString::number(results.size()) + " files";
	if(hasDeleted)
	{
		statusText += " WARNING: Some files don't exist anymore. No preview available for those. Index out of sync";
	}
	ui->lblSearchResults->setText(statusText);
}

void MainWindow::makePreviews(int page)
{
	if(this->previewableSearchResults.empty())
	{
		return;
	}
	qDeleteAll(ui->scrollAreaWidgetContents->children());

	ui->scrollAreaWidgetContents->setLayout(new QHBoxLayout());
	ui->previewProcessBar->setMaximum(this->previewableSearchResults.size());
	processedPdfPreviews = 0;
	QString scaleText = ui->comboScale->currentText();
	scaleText.chop(1);

	QVector<QString> wordsToHighlight;
	QRegularExpression extractor(R"#("([^"]*)"|(\w+))#");
	for(const Token &token : this->contentSearchQuery.getTokens())
	{
		if(token.type == FILTER_CONTENT_CONTAINS)
		{
			QRegularExpressionMatchIterator i = extractor.globalMatch(token.value);
			while(i.hasNext())
			{
				QRegularExpressionMatch m = i.next();
				QString value = m.captured(1);
				if(value.isEmpty())
				{
					value = m.captured(2);
				}
				wordsToHighlight.append(value);
			}
		}
	}
	int end = previewsPerPage;
	int begin = page * previewsPerPage - previewsPerPage;

	RenderConfig renderConfig;
	renderConfig.scaleX = QGuiApplication::primaryScreen()->physicalDotsPerInchX() * (scaleText.toInt() / 100.);
	renderConfig.scaleY = QGuiApplication::primaryScreen()->physicalDotsPerInchY() * (scaleText.toInt() / 100.);
	renderConfig.wordsToHighlight = wordsToHighlight;

	QVector<RenderTarget> targets;
	for(SearchResult &sr : this->previewableSearchResults.mid(begin, end))
	{
		RenderTarget renderTarget;
		renderTarget.path = sr.fileData.absPath;

		for(unsigned int pagenum : sr.pages)
		{
			renderTarget.page = (int)pagenum;
			targets.append(renderTarget);
		}
	}
	ui->previewProcessBar->setMaximum(targets.count());
	ui->previewProcessBar->setMinimum(0);
	ui->previewProcessBar->setValue(0);
	ui->previewProcessBar->setVisible(this->previewableSearchResults.size() > 0);
	++this->currentPreviewGeneration;
	emit startIpcPreviews(renderConfig, targets);
}

void MainWindow::handleSearchError(QString error)
{
	ui->lblSearchResults->setText("Error:" + error);
}

void MainWindow::createSearchResutlMenu(QMenu &menu, const QFileInfo &fileInfo)
{
	menu.addAction("Copy filename to clipboard",
				   [&fileInfo] { QGuiApplication::clipboard()->setText(fileInfo.fileName()); });
	menu.addAction("Copy full path to clipboard",
				   [&fileInfo] { QGuiApplication::clipboard()->setText(fileInfo.absoluteFilePath()); });
	menu.addAction("Open containing folder", [this, &fileInfo] { this->ipcFileOpen(fileInfo.absolutePath()); });
}

void MainWindow::ipcDocOpen(QString path, int num)
{
	QStringList args;
	args << path;
	args << QString::number(num);
	// this->ipcClient->sendCommand(DocOpen, args);
}

void MainWindow::ipcFileOpen(QString path)
{
	QStringList args;
	args << path;
	// this->ipcClient->sendCommand(FileOpen, args);
}

void MainWindow::treeSearchItemActivated(QTreeWidgetItem *item, int i)
{
	ipcFileOpen(item->text(1));
}

void MainWindow::showSearchResultsContextMenu(const QPoint &point)
{
	QTreeWidgetItem *item = ui->treeResultsList->itemAt(point);
	if(item == nullptr)
	{
		return;
	}
	QFileInfo pathinfo(item->text(1));
	QMenu menu("SearchResults", this);
	createSearchResutlMenu(menu, pathinfo);
	menu.exec(QCursor::pos());
}

MainWindow::~MainWindow()
{
	delete ui;
}
