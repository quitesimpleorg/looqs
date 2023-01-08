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
#include <QDesktopWidget>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clicklabel.h"
#include "../shared/sqlitesearch.h"
#include "../shared/looqsgeneralexception.h"
#include "../shared/common.h"
#include "previewgenerator.h"
#include "aboutdialog.h"

MainWindow::MainWindow(QWidget *parent, QString socketPath)
	: QMainWindow(parent), ui(new Ui::MainWindow), progressDialog(this)
{
	this->progressDialog.cancel(); // because constructing it shows it, quite weird
	ui->setupUi(this);
	setWindowTitle(QCoreApplication::applicationName());

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

	QStringList searchHistoryList = settings.value(SETTINGS_KEY_SEARCHHISTORY).toStringList();
	this->searchHistory = searchHistoryList.toVector();
	this->currentSearchHistoryIndex = this->searchHistory.size();

	ui->spinPreviewPage->setValue(1);
	ui->spinPreviewPage->setMinimum(1);

	ui->btnOpenFailed->setVisible(false);
	ui->comboPreviewFiles->setVisible(false);

	auto policy = ui->btnOpenFailed->sizePolicy();
	policy.setRetainSizeWhenHidden(true);
	ui->btnOpenFailed->setSizePolicy(policy);

	ui->txtSearch->installEventFilter(this);
	ui->scrollArea->viewport()->installEventFilter(this);

	this->previewCoordinator.setSocketPath(socketPath);
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
	connect(ui->menuOpenUserManualAction, &QAction::triggered, this,
			[this]() { QDesktopServices::openUrl(Common::userManualUrl()); });

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
		ui->comboPreviewFiles, qOverload<int>(&QComboBox::currentIndexChanged), this,
		[&]()
		{
			if(this->previewTabActive())
			{
				makePreviews(1);
			}
		},
		Qt::QueuedConnection);
	connect(&previewCoordinator, &PreviewCoordinator::previewReady, this, &MainWindow::previewReceived,
			Qt::QueuedConnection);
	connect(&previewCoordinator, &PreviewCoordinator::completedGeneration, this,
			[&]
			{
				this->ui->previewProcessBar->setValue(this->ui->previewProcessBar->maximum());
				this->ui->spinPreviewPage->setEnabled(true);
				this->ui->comboPreviewFiles->setEnabled(true);
				ui->txtSearch->setEnabled(true);
			});
	connect(&previewCoordinator, &PreviewCoordinator::error, this,
			[this](QString msg)
			{
				qCritical() << msg << Qt::endl;
				QMessageBox::critical(this, "IPC error", msg);
			});

	/*connect(this, &MainWindow::startIpcPreviews, &previewCoordinator, &IPCPreviewClient::startGeneration,
			Qt::QueuedConnection);
	connect(this, &MainWindow::stopIpcPreviews, &ipcPreviewClient, &IPCPreviewClient::stopGeneration,
			Qt::QueuedConnection); */
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

void MainWindow::processShortcut(int key)
{
	if(key == Qt::Key_Tab || key == Qt::Key_Backtab)
	{
		int tabIndex = ui->tabWidget->currentIndex();
		if(key == Qt::Key_Tab)
		{
			++tabIndex;
		}
		if(key == Qt::Key_Backtab)
		{
			--tabIndex;
		}
		tabIndex = tabIndex % ui->tabWidget->count();
		if(tabIndex < 0)
		{
			tabIndex = ui->tabWidget->count() - 1;
		}
		ui->tabWidget->setCurrentIndex(tabIndex);
	}
	if(key == Qt::Key_L)
	{
		ui->txtSearch->setFocus();
		ui->txtSearch->selectAll();
	}
	if(key == Qt::Key_W)
	{
		ui->txtSearch->setFocus();
		QString currentText = ui->txtSearch->text().trimmed();
		int index = currentText.lastIndexOf(QRegularExpression("[\\s\\)]"));
		if(index != -1)
		{
			bool isFilter = (index == currentText.length() - 1);
			currentText.remove(index + 1, currentText.length() - index - 1);
			if(isFilter)
			{
				index = currentText.lastIndexOf(' ', index);
				if(index == -1)
				{
					index = 0;
				}
				currentText.remove(index, currentText.length());
			}
			if(currentText.length() > 0)
			{
				currentText += ' ';
			}
			ui->txtSearch->setText(currentText);
		}
		else
		{
			ui->txtSearch->clear();
		}
	}
	if(key == Qt::Key_F)
	{
		ui->txtSearch->setFocus();
		QString currentText = ui->txtSearch->text().trimmed();
		int index = currentText.lastIndexOf(')');
		if(index != -1)
		{
			bool isFilter = (index == currentText.length() - 1);
			if(!isFilter)
			{
				ui->txtSearch->setSelection(index + 2, ui->txtSearch->text().length() - index - 1);
			}
			else
			{
				int begin = currentText.lastIndexOf('(', index - 1);
				if(begin != -1)
				{
					ui->txtSearch->setSelection(begin + 1, index - begin - 1);
				}
			}
		}
		else
		{
			int spaceIndex = currentText.lastIndexOf(' ');
			int colonIndex = currentText.lastIndexOf(':');
			if(colonIndex > spaceIndex)
			{
				int target = currentText.indexOf(' ', colonIndex);
				if(target == -1)
				{
					target = ui->txtSearch->text().size() - colonIndex;
				}
				ui->txtSearch->setSelection(colonIndex + 1, target - 1);
			}
			else
			{
				ui->txtSearch->setSelection(spaceIndex + 1, ui->txtSearch->text().size() - spaceIndex - 1);
			}
		}
	}
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	if(object == ui->txtSearch && !searchHistory.empty())
	{
		if(event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if(keyEvent->key() == Qt::Key_Up)
			{
				if(this->currentSavedSearchText.isEmpty())
				{
					this->currentSavedSearchText = ui->txtSearch->text();
				}
				if(this->currentSearchHistoryIndex <= 0)
				{
					return true;
				}
				--this->currentSearchHistoryIndex;
				QString text = this->searchHistory.at(this->currentSearchHistoryIndex);
				ui->txtSearch->setText(text);
				return true;
			}
			else if(keyEvent->key() == Qt::Key_Down)
			{
				if(this->currentSearchHistoryIndex == searchHistory.size() - 1)
				{
					if(!this->currentSavedSearchText.isEmpty())
					{
						ui->txtSearch->setText(this->currentSavedSearchText);
						this->currentSavedSearchText.clear();
						++this->currentSearchHistoryIndex;
					}
					return true;
				}
				if(this->currentSearchHistoryIndex < searchHistory.size() - 1)
				{
					++this->currentSearchHistoryIndex;
					QString text = this->searchHistory.at(this->currentSearchHistoryIndex);
					ui->txtSearch->setText(text);
				}
				return true;
			}
			else
			{
				this->currentSavedSearchText.clear();
				/* Off by one on purpose so Key_Up decrements it again and lands at
				 * the last entry */
				this->currentSearchHistoryIndex = this->searchHistory.size();
			}
		}
	}
	if(object == ui->scrollArea->viewport())
	{
		if(event->type() == QEvent::Wheel)
		{
			QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
			if(wheelEvent->modifiers() & Qt::ControlModifier)
			{
				if(wheelEvent->angleDelta().y() > 0)
				{
					if(ui->comboScale->currentIndex() < ui->comboScale->count() - 1)
					{
						ui->comboScale->setCurrentIndex(ui->comboScale->currentIndex() + 1);
					}
				}

				else
				{
					if(ui->comboScale->currentIndex() > 0)
					{
						ui->comboScale->setCurrentIndex(ui->comboScale->currentIndex() - 1);
					}
				}
				return true;
			}
		}
	}
	return QMainWindow::eventFilter(object, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	bool quit =
		((event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Q) || event->key() == Qt::Key_Escape);
	if(quit)
	{
		qApp->quit();
		return;
	}

	if(event->modifiers() & Qt::ControlModifier)
	{
		processShortcut(event->key());
		return;
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
	bool horizontalScroll = settings.value(SETTINGS_KEY_PREVIEWS_SCROLL_HORIZONTALLY).toBool();
	ui->radioScrollHorizontally->setChecked(horizontalScroll);
	ui->radioScrollVertically->setChecked(!horizontalScroll);
}

void MainWindow::saveSettings()
{
	if(ui->chkRemoveOldDb->isChecked())
	{
		bool result = QFile::remove(Common::databasePath());
		if(!result)
		{
			QMessageBox::critical(this, "Error removing database",
								  "Failed to remove old database. Settings not saved.");
			return;
		}
	}

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
	settings.setValue(SETTINGS_KEY_PREVIEWS_SCROLL_HORIZONTALLY, ui->radioScrollHorizontally->isChecked());

	settings.sync();

	QProcess::startDetached(qApp->arguments()[0], qApp->arguments().mid(1));
	qApp->quit();
}

void MainWindow::previewReceived()
{
	this->ui->previewProcessBar->setValue(this->ui->previewProcessBar->value() + 1);
	QBoxLayout *layout = static_cast<QBoxLayout *>(ui->scrollAreaWidgetContents->layout());
	int index = layout->count();
	if(index > 0)
	{
		--index;
	}
	QSharedPointer<PreviewResult> preview = this->previewCoordinator.resultAt(index);

	if(!preview.isNull() && preview->hasPreview())
	{
		QString docPath = preview->getDocumentPath();
		auto previewPage = preview->getPage();
		ClickLabel *headerLabel = new ClickLabel();
		headerLabel->setText(QString("Path: ") + preview->getDocumentPath());

		ClickLabel *label = dynamic_cast<ClickLabel *>(preview->createPreviewWidget());
		label->setMaximumWidth(QApplication::desktop()->availableGeometry().width() - 200);

		QVBoxLayout *previewLayout = new QVBoxLayout();

		QFont font = label->font();
		font.setPointSize(QApplication::font().pointSize() * currentSelectedScale() / 100);
		label->setFont(font);
		headerLabel->setFont(font);

		auto leftClickHandler = [this, docPath, previewPage]() { openDocument(docPath, previewPage); };
		auto rightClickhandler = [this, docPath, previewPage]()
		{
			QFileInfo fileInfo{docPath};
			QMenu menu("labeRightClick", this);
			createSearchResultMenu(menu, fileInfo);
			menu.addAction("Copy page number",
						   [previewPage] { QGuiApplication::clipboard()->setText(QString::number(previewPage)); });
			menu.exec(QCursor::pos());
		};

		connect(label, &ClickLabel::leftClick, leftClickHandler);
		connect(label, &ClickLabel::rightClick, rightClickhandler);

		connect(headerLabel, &ClickLabel::rightClick, rightClickhandler);

		previewLayout->addWidget(label);

		previewLayout->addWidget(headerLabel);

		previewLayout->setMargin(0);
		previewLayout->insertStretch(0, 1);
		previewLayout->insertStretch(-1, 1);
		previewLayout->setAlignment(Qt::AlignCenter);
		QWidget *previewWidget = new QWidget();

		previewWidget->setLayout(previewLayout);

		layout->insertWidget(index, previewWidget);
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
	while(this->searchHistory.size() > 30)
	{
		this->searchHistory.removeFirst();
	}
	this->searchHistory.append(q);
	this->currentSearchHistoryIndex = this->searchHistory.size();
	this->currentSavedSearchText.clear();

	ui->treeResultsList->clear();
	ui->lblSearchResults->setText("Searching...");
	this->ui->txtSearch->setEnabled(false);
	QFuture<QVector<SearchResult>> searchFuture = QtConcurrent::run(
		[&, q]()
		{
			SqliteSearch searcher(db);
			QVector<SearchResult> results;
			LooqsQuery tmpQuery = LooqsQuery::build(q, TokenType::WORD, true);

			LooqsQuery pathsQuery = tmpQuery;

			this->contentSearchQuery = tmpQuery;

			bool addContentSearch = false;
			bool addPathSearch = false;

			auto createFinalTokens = [&tmpQuery](TokenType replacementToken)
			{
				QVector<Token> tokens = tmpQuery.getTokens();
				for(Token &token : tokens)
				{
					if(token.type == TokenType::WORD)
					{
						token.type = replacementToken;
					}
				}
				return tokens;
			};

			/* An explicit search, no lone words. We just pass it on */
			if(!(tmpQuery.getTokensMask() & TokenType::WORD))
			{
				if(tmpQuery.hasContentSearch())
				{
					addContentSearch = true;
				}
				else
				{
					addPathSearch = true;
				}
			}
			/* A path search, and lone words, e. g. p:("docs") invoice */
			else if(tmpQuery.hasPathSearch() && (tmpQuery.getTokensMask() & TokenType::WORD))
			{
				this->contentSearchQuery = tmpQuery;
				this->contentSearchQuery.setTokens(createFinalTokens(TokenType::FILTER_CONTENT_CONTAINS));
				addContentSearch = true;
				addPathSearch = false;
			}
			/* A content search and lone words, e. g. c:("to be or not") ebooks */
			else if(tmpQuery.hasContentSearch() && (tmpQuery.getTokensMask() & TokenType::WORD))
			{
				this->contentSearchQuery = LooqsQuery::build(q, TokenType::FILTER_PATH_CONTAINS, false);
				addContentSearch = true;
				addPathSearch = false;
			}
			/* "Simply lone words, so search both" */
			else if(!tmpQuery.hasPathSearch() && !tmpQuery.hasContentSearch())
			{
				this->contentSearchQuery.setTokens(createFinalTokens(TokenType::FILTER_CONTENT_CONTAINS));
				pathsQuery = LooqsQuery::build(q, TokenType::FILTER_PATH_CONTAINS, false);
				addContentSearch = true;
				addPathSearch = true;
			}
			if(addPathSearch)
			{
				if(pathsQuery.getLimit() == -1)
				{
					pathsQuery.setLimit(1000);
				}
				results.append(searcher.search(pathsQuery));
			}
			if(addContentSearch)
			{
				if(this->contentSearchQuery.getLimit() == -1)
				{
					this->contentSearchQuery.setLimit(1000);
				}
				results.append(searcher.search(this->contentSearchQuery));
			}
			else
			{
				this->contentSearchQuery.setTokens({});
			}
			return results;
		});
	searchWatcher.setFuture(searchFuture);
}

void MainWindow::handleSearchResults(const QVector<SearchResult> &results)
{
	qDeleteAll(ui->scrollAreaWidgetContents->children());

	ui->treeResultsList->clear();
	ui->comboPreviewFiles->clear();
	ui->comboPreviewFiles->addItem("All previews");
	ui->comboPreviewFiles->setVisible(true);
	ui->lblTotalPreviewPagesCount->setText("");

	this->previewCoordinator.init(results);

	bool hasDeleted = false;
	QHash<QString, bool> seenMap;
	for(const SearchResult &result : results)
	{
		const QString &absPath = result.fileData.absPath;
		QFileInfo pathInfo(absPath);

		if(!seenMap.contains(absPath))
		{
			QString fileName = pathInfo.fileName();
			QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeResultsList);

			QDateTime dt = QDateTime::fromSecsSinceEpoch(result.fileData.mtime);
			item->setIcon(0, iconProvider.icon(pathInfo));
			item->setText(0, fileName);
			item->setText(1, absPath);
			item->setText(2, dt.toString(Qt::ISODate));
			item->setText(3, this->locale().formattedDataSize(result.fileData.size));
		}
		bool exists = pathInfo.exists();
		if(!exists)
		{
			hasDeleted = true;
		}
		seenMap[absPath] = true;
	}

	seenMap.clear();
	for(const SearchResult &result : this->previewCoordinator.getPreviewableSearchResults())
	{
		const QString &absPath = result.fileData.absPath;
		if(!seenMap.contains(absPath))
		{
			ui->comboPreviewFiles->addItem(absPath);
		}
		seenMap[absPath] = true;
	}

	ui->treeResultsList->resizeColumnToContents(0);
	ui->treeResultsList->resizeColumnToContents(1);
	ui->treeResultsList->resizeColumnToContents(2);

	previewDirty = this->previewCoordinator.previewableCount() > 0;

	ui->spinPreviewPage->setValue(1);

	if(previewTabActive() && previewDirty)
	{
		makePreviews(1);
	}

	QString statusText = "Results: " + QString::number(results.size()) + " files";
	statusText += ", previewable: " + QString::number(this->previewCoordinator.previewableCount());
	if(hasDeleted)
	{
		statusText += " WARNING: Some files are inaccessible. No preview available for those. Index may be out of sync";
	}
	ui->lblSearchResults->setText(statusText);
}

int MainWindow::currentSelectedScale()
{
	QString scaleText = ui->comboScale->currentText();
	scaleText.chop(1);
	return scaleText.toInt();
}

void MainWindow::makePreviews(int page)
{
	if(this->previewCoordinator.previewableCount() == 0)
	{
		return;
	}
	qDeleteAll(ui->scrollAreaWidgetContents->children());

	QSettings settings;
	bool horizontalScroll = settings.value(SETTINGS_KEY_PREVIEWS_SCROLL_HORIZONTALLY, false).toBool();
	if(horizontalScroll)
	{
		ui->scrollAreaWidgetContents->setLayout(new QHBoxLayout());
	}
	else
	{
		ui->scrollAreaWidgetContents->setLayout(new QVBoxLayout());
		ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignCenter);
	}
	ui->previewProcessBar->setMaximum(this->previewCoordinator.previewableCount());

	QVector<QString> wordsToHighlight;
	QRegularExpression extractor(R"#("([^"]*)"|([^\s]+))#");
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
	int length = previewsPerPage;
	int beginOffset = page * previewsPerPage - previewsPerPage;
	if(beginOffset < 0)
	{
		// Should not happen actually
		beginOffset = 0;
	}

	int currentScale = currentSelectedScale();
	RenderConfig renderConfig;
	renderConfig.scaleX = QGuiApplication::primaryScreen()->physicalDotsPerInchX() * (currentScale / 100.);
	renderConfig.scaleY = QGuiApplication::primaryScreen()->physicalDotsPerInchY() * (currentScale / 100.);
	renderConfig.wordsToHighlight = wordsToHighlight;

	int previewPos = 0;
	QVector<RenderTarget> targets;
	for(const SearchResult &sr : this->previewCoordinator.getPreviewableSearchResults())
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
		renderTarget.page = (int)sr.page;

		targets.append(renderTarget);
	}
	int numpages = ceil(static_cast<double>(targets.size()) / previewsPerPage);
	ui->spinPreviewPage->setMaximum(numpages);
	targets = targets.mid(beginOffset, length);

	ui->lblTotalPreviewPagesCount->setText(QString::number(numpages));
	ui->previewProcessBar->setMaximum(targets.count());
	ui->previewProcessBar->setMinimum(0);
	ui->previewProcessBar->setValue(0);
	ui->previewProcessBar->setVisible(this->previewCoordinator.previewableCount() > 0);
	this->ui->spinPreviewPage->setEnabled(false);
	this->ui->comboPreviewFiles->setEnabled(false);
	this->ui->txtSearch->setEnabled(false);

	this->previewCoordinator.startGeneration(renderConfig, targets);
}

void MainWindow::handleSearchError(QString error)
{
	ui->lblSearchResults->setText("Error:" + error);
}

void MainWindow::createSearchResultMenu(QMenu &menu, const QFileInfo &fileInfo)
{
	menu.addAction("Copy filename to clipboard",
				   [&fileInfo] { QGuiApplication::clipboard()->setText(fileInfo.fileName()); });
	menu.addAction("Copy full path to clipboard",
				   [&fileInfo] { QGuiApplication::clipboard()->setText(fileInfo.absoluteFilePath()); });
	menu.addAction("Open containing folder", [this, &fileInfo] { this->openFile(fileInfo.absolutePath()); });

	auto previewables = this->previewCoordinator.getPreviewableSearchResults();
	auto result =
		std::find_if(previewables.begin(), previewables.end(),
					 [this, &fileInfo](SearchResult &a) { return fileInfo.absoluteFilePath() == a.fileData.absPath; });

	if(result != previewables.end())
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
	createSearchResultMenu(menu, pathinfo);
	menu.exec(QCursor::pos());
}

MainWindow::~MainWindow()
{
	syncerThread.terminate();
	delete this->indexSyncer;
	delete this->dbService;
	delete this->dbFactory;
	delete this->indexer;
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	QStringList list = this->searchHistory.toList();
	QSettings settings;
	settings.setValue(SETTINGS_KEY_SEARCHHISTORY, list);
	settings.sync();
}
