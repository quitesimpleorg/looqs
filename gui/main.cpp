#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QStandardPaths>
#include "mainwindow.h"
#include "searchresult.h"
#include "pdfpreview.h"
#include "../shared/common.h"
#include "../submodules/qssb.h/qssb.h"

int main(int argc, char *argv[])
{
	struct qssb_policy *policy = qssb_init_policy();
	std::string appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString();
	std::string cacheDataLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation).toStdString();

	policy->namespace_options = QSSB_UNSHARE_NETWORK | QSSB_UNSHARE_USER;
	qssb_append_path_policy(policy, QSSB_FS_ALLOW_READ | QSSB_FS_ALLOW_REMOVE_FILE, "/");
	qssb_append_path_policy(policy, QSSB_FS_ALLOW_READ | QSSB_FS_ALLOW_REMOVE_FILE | QSSB_FS_ALLOW_WRITE,
							appDataLocation.c_str());
	qssb_append_path_policy(policy, QSSB_FS_ALLOW_READ | QSSB_FS_ALLOW_REMOVE_FILE | QSSB_FS_ALLOW_WRITE,
							cacheDataLocation.c_str());

	int ret = qssb_enable_policy(policy);
	if(ret != 0)
	{
		qDebug() << "Failed to establish sandbox";
		return 1;
	}
	qssb_free_policy(policy);

	Common::setupAppInfo();
	QApplication a(argc, argv);
	try
	{
		Common::ensureConfigured();
	}
	catch(LooqsGeneralException &e)
	{
		qDebug() << e.message;
		QMessageBox::critical(nullptr, "Error", e.message);
		return 1;
	}
	qRegisterMetaType<QVector<SearchResult>>("QVector<SearchResult>");
	qRegisterMetaType<QVector<PdfPreview>>("QVector<PdfPreview>");
	qRegisterMetaType<PdfPreview>("PdfPreview");
	MainWindow w;
	w.showMaximized();

	return a.exec();
}
