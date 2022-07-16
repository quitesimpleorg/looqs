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
#include <QProgressDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clicklabel.h"
#include "../shared/sqlitesearch.h"
#include "../shared/looqsgeneralexception.h"
#include "../shared/common.h"
#include "ipcpreviewclient.h"
#include "previewgenerator.h"
#include "aboutdialog.h"

MainWindow::MainWindow(QWidget *parent, QString socketPath)
	: QMainWindow(parent), ui(new Ui::MainWindow), progressDialog(this)
{
	this->progressDialog.cancel(); // because constructing it shows it, quite weird
	ui->setupUi(this);
	setWindowTitle(QCoreApplication::applicationName());
	this->ipcPreviewClient.moveToThread(&this->ipcClientThread);
	this->ipcPreviewClient.setSocketPath(socketPath);
	QSettings settings;

	this->dbFactory = new DatabaseFactory(Common::databasePath());

	db = this->dbFactory->forCurrentThread();
	this->dbService = new SqliteDbService(*this->dbFactory);
	this->indexSyncer = new IndexSyncer(*this->dbService);
	this->indexSyncer->moveToThread(&this->syncerThread);

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

	QString ignorePatterns = settings.value("ignorePatterns").toString();
	ui->txtIgnorePatterns->setText(ignorePatterns);

	ui->spinPreviewPage->setValue(1);
	ui->spinPreviewPage->setMinimum(1);

	ui->btnOpenFailed->setVisible(false);
	ui->comboPreviewFiles->setVisible(false);

	auto policy = ui->btnOpenFailed->sizePolicy();
	policy.setRetainSizeWhenHidden(true);
	ui->btnOpenFailed->setSizePolicy(policy);

	this->ipcClientThread.start();
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
	connect(ui->menuAboutAction, &QAction::triggered, this,
			[this](bool checked)
			{
				AboutDialog aboutDialog(this);

				aboutDialog.exec();
			});
	connect(ui->menuAboutQtAction, &QAction::triggered, this,
			[this](bool checked) { QMessageBox::aboutQt(this, "About Qt"); });
	connect(ui->menuSyncIndexAction, &QAction::triggered, this, &MainWindow::startIndexSync);
	connect(indexSyncer, &IndexSyncer::finished, this,
			[&](unsigned int totalUpdated, unsigned int totalDeleted, unsigned int totalErrored)
			{
				this->progressDialog.cancel();

				QMessageBox::information(
					this, "Syncing finished",
					QString("Syncing finished\n\nTotal updated: %1\nTotal deleted: %2\nTotal errors: %3\n")
						.arg(QString::number(totalUpdated))
						.arg(QString::number(totalDeleted))
						.arg(QString::number(totalErrored)));
			});
	connect(this, &MainWindow::beginIndexSync, indexSyncer, &IndexSyncer::sync);
	connect(&this->progressDialog, &QProgressDialog::canceled, indexSyncer, &IndexSyncer::cancel);
	connect(ui->btnSaveSettings, &QPushButton::clicked, this, &MainWindow::saveSettings);
	connect(ui->btnOpenFailed, &QPushButton::clicked, this, &MainWindow::exportFailedPaths);
	connect(
		ui->comboPreviewFiles, qOverload<int>(&QComboBox::currentIndexChanged), this, [&]() { makePreviews(1); },
		Qt::QueuedConnection);
	connect(&ipcPreviewClient, &IPCPreviewClient::previewReceived, this, &MainWindow::previewReceived,
			Qt::QueuedConnection);
	connect(&ipcPreviewClient, &IPCPreviewClient::finished, this,
			[&]
			{
				this->ui->previewProcessBar->setValue(this->ui->previewProcessBar->maximum());
				this->ui->spinPreviewPage->setEnabled(true);
				this->ui->comboPreviewFiles->setEnabled(true);
			});
	connect(&ipcPreviewClient, &IPCPreviewClient::error, this,
			[this](QString msg)
			{
				qCritical() << msg << Qt::endl;
				QMessageBox::critical(this, "IPC error", msg);
			});

	connect(this, &MainWindow::startIpcPreviews, &ipcPreviewClient, &IPCPreviewClient::startGeneration,
			Qt::QueuedConnection);
	connect(this, &MainWindow::stopIpcPreviews, &ipcPreviewClient, &IPCPreviewClient::stopGeneration,
			Qt::QueuedConnection);
}

void MainWindow::exportFailedPaths()
{

	QString filename =
		QString("/tmp/looqs_indexresult_failed_%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmmss"));
	QFile outFile(filename);
	if(!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Failed to write log file", "An error occured while trying to create log file");
		return;
	}

	QTextStream stream(&outFile);

	IndexResult result = this->indexer->getResult();

	stream << "Failed to index the following paths:\n";
	for(FileScanResult &fsr : result.failedResults())
	{
		stream << fsr.first << " " << SaveFileResultToString(fsr.second) << '\n';
	}

	QDesktopServices::openUrl(filename);
}

void MainWindow::startIndexSync()
{
	progressDialog.setWindowTitle("Syncing");
	progressDialog.setLabelText("Syncing - this might take a moment, please wait");
	progressDialog.setWindowModality(Qt::ApplicationModal);
	progressDialog.setMinimum(0);
	progressDialog.setMaximum(0);
	progressDialog.setValue(0);
	progressDialog.open();

	indexSyncer->setKeepGoing(true);
	indexSyncer->setVerbose(false);
	indexSyncer->setDryRun(false);
	indexSyncer->setRemoveDeletedFromIndex(true);

	this->syncerThread.start();

	emit beginIndexSync();
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
	ui->settingsTab->setEnabled(false);
	ui->txtPathScanAdd->setEnabled(false);
	ui->txtSearch->setEnabled(false);
	ui->previewProcessBar->setValue(0);
	ui->previewProcessBar->setVisible(true);
	ui->btnOpenFailed->setVisible(false);

	QVector<QString> paths;
	QStringList pathSettingsValue;
	for(int i = 0; i < ui->lstPaths->count(); i++)
	{
		QString path = ui->lstPaths->item(i)->text();
		paths.append(path);
		pathSettingsValue.append(path);
	}
	this->indexer->setTargetPaths(paths);
	QString ignorePatterns = ui->txtIgnorePatterns->text();
	this->indexer->setIgnorePattern(ignorePatterns.split(";"));
	this->indexer->beginIndexing();
	QSettings settings;
	settings.setValue("indexPaths", pathSettingsValue);
	settings.setValue("ignorePatterns", ignorePatterns);
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
	ui->settingsTab->setEnabled(true);
	ui->txtPathScanAdd->setEnabled(true);
	ui->txtSearch->setEnabled(true);
	if(result.erroredPaths > 0)
	{
		ui->btnOpenFailed->setVisible(true);
	}
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
	/* Settings tab active */
	if(ui->tabWidget->currentIndex() == 3)
	{
		initSettingsTabs();
	}
}

void MainWindow::initSettingsTabs()
{
	QSettings settings;

	QString pdfViewerCmd = settings.value(SETTINGS_KEY_PDFVIEWER).toString();
	QString excludedPaths = Common::excludedPaths().join(';');
	QString mountPaths = Common::mountPaths().join(';');
	QString databasePath = Common::databasePath();
	int numPagesPerPreview = settings.value(SETTINGS_KEY_PREVIEWSPERPAGE, 20).toInt();

	ui->txtSettingPdfPreviewerCmd->setText(pdfViewerCmd);
	ui->txtSettingIgnoredPaths->setText(excludedPaths);
	ui->txtSettingMountPaths->setText(mountPaths);
	ui->spinSettingNumerPerPages->setValue(numPagesPerPreview);
	ui->txtSettingDatabasePath->setText(databasePath);
}

void MainWindow::saveSettings()
{
	QSettings settings;

	QString pdfViewerCmd = ui->txtSettingPdfPreviewerCmd->text();
	QStringList excludedPaths = ui->txtSettingIgnoredPaths->text().split(';');
	QStringList mountPaths = ui->txtSettingMountPaths->text().split(';');
	QString databasePath = ui->txtSettingDatabasePath->text();

	settings.setValue(SETTINGS_KEY_PDFVIEWER, pdfViewerCmd);
	settings.setValue(SETTINGS_KEY_EXCLUDEDPATHS, excludedPaths);
	settings.setValue(SETTINGS_KEY_MOUNTPATHS, mountPaths);
	settings.setValue(SETTINGS_KEY_PREVIEWSPERPAGE, ui->spinSettingNumerPerPages->value());
	settings.setValue(SETTINGS_KEY_DBPATH, databasePath);

	settings.sync();

	QProcess::startDetached(qApp->arguments()[0], qApp->arguments().mid(1));
	qApp->quit();
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
		connect(label, &ClickLabel::leftClick, [this, docPath, previewPage]() { openDocument(docPath, previewPage); });
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
	if(ui->tabWidget->currentIndex() > 1)
	{
		ui->tabWidget->setCurrentIndex(0);
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
				if(filesQuery.getLimit() == -1)
				{
					filesQuery.setLimit(1000);
				}

				results.append(searcher.search(filesQuery));
			}
			if(addContentSearch)
			{
				if(this->contentSearchQuery.getLimit() == -1)
				{
					this->contentSearchQuery.setLimit(1000);
				}

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
	ui->comboPreviewFiles->clear();
	ui->comboPreviewFiles->addItem("All previews");
	ui->comboPreviewFiles->setVisible(true);

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
			if(!result.wasContentSearch)
			{
				continue;
			}

			if(!pathInfo.suffix().contains("htm")) // hack until we can preview them properly...
			{
				if(PreviewGenerator::get(pathInfo) != nullptr)
				{
					this->previewableSearchResults.append(result);
					ui->comboPreviewFiles->addItem(result.fileData.absPath);
				}
			}
		}
		else
		{
			hasDeleted = true;
		}
	}
	ui->treeResultsList->resizeColumnToContents(0);
	ui->treeResultsList->resizeColumnToContents(1);
	ui->treeResultsList->resizeColumnToContents(2);
	previewDirty = !this->previewableSearchResults.empty();

	ui->spinPreviewPage->setValue(1);

	if(previewTabActive() && previewDirty)
	{
		makePreviews(1);
	}

	QString statusText = "Results: " + QString::number(results.size()) + " files";
	if(hasDeleted)
	{
		statusText += " WARNING: Some files are inaccessible. No preview available for those. Index may be out of sync";
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
	QRegularExpression extractor(R"#("([^"]*)"|((\p{L}|\p{N})+))#");
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
	if(begin < 0)
	{
		// Should not happen actually
		begin = 0;
	}

	RenderConfig renderConfig;
	renderConfig.scaleX = QGuiApplication::primaryScreen()->physicalDotsPerInchX() * (scaleText.toInt() / 100.);
	renderConfig.scaleY = QGuiApplication::primaryScreen()->physicalDotsPerInchY() * (scaleText.toInt() / 100.);
	renderConfig.wordsToHighlight = wordsToHighlight;

	QVector<RenderTarget> targets;
	for(SearchResult &sr : this->previewableSearchResults)
	{
		if(ui->comboPreviewFiles->currentIndex() != 0)
		{
			if(sr.fileData.absPath != ui->comboPreviewFiles->currentText())
			{
				continue;
			}
		}
		RenderTarget renderTarget;
		renderTarget.path = sr.fileData.absPath;

		for(unsigned int pagenum : sr.pages)
		{
			renderTarget.page = (int)pagenum;
			targets.append(renderTarget);
		}
	}
	int numpages = ceil(static_cast<double>(targets.size()) / previewsPerPage);
	ui->spinPreviewPage->setMaximum(numpages);
	targets = targets.mid(begin, end);

	ui->lblTotalPreviewPagesCount->setText(QString::number(numpages));
	ui->previewProcessBar->setMaximum(targets.count());
	ui->previewProcessBar->setMinimum(0);
	ui->previewProcessBar->setValue(0);
	ui->previewProcessBar->setVisible(this->previewableSearchResults.size() > 0);
	++this->currentPreviewGeneration;
	this->ui->spinPreviewPage->setEnabled(false);
	this->ui->comboPreviewFiles->setEnabled(false);
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
	menu.addAction("Open containing folder", [this, &fileInfo] { this->openFile(fileInfo.absolutePath()); });

	auto result =
		std::find_if(this->previewableSearchResults.begin(), this->previewableSearchResults.end(),
					 [this, &fileInfo](SearchResult &a) { return fileInfo.absoluteFilePath() == a.fileData.absPath; });

	if(result != this->previewableSearchResults.end())
	{
		menu.addAction("Show previews for this file",
					   [this, &fileInfo]
					   {
						   ui->tabWidget->setCurrentIndex(1);
						   this->ui->comboPreviewFiles->setCurrentText(fileInfo.absoluteFilePath());
					   });
	}
}

void MainWindow::openDocument(QString path, int num)
{
	QSettings settings;
	QString command = settings.value("pdfviewer").toString();
	if(path.endsWith(".pdf") && command != "" && command.contains("%p") && command.contains("%f"))
	{
		QStringList splitted = command.split(" ");
		if(splitted.size() > 1)
		{
			QString cmd = splitted[0];
			QStringList args = splitted.mid(1);
			args.replaceInStrings("%f", path);
			args.replaceInStrings("%p", QString::number(num));
			QProcess::startDetached(cmd, args);
		}
	}
	else
	{
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}
}

void MainWindow::openFile(QString path)
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::treeSearchItemActivated(QTreeWidgetItem *item, int i)
{
	openFile(item->text(1));
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
	syncerThread.terminate();
	ipcClientThread.terminate();
	delete this->indexSyncer;
	delete this->dbService;
	delete this->dbFactory;
	delete this->indexer;
	delete ui;
}
