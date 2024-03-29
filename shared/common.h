#ifndef COMMON_H
#define COMMON_H
#include <QCoreApplication>
#include <QFileInfo>

#define SETTINGS_KEY_DBPATH "dbpath"
#define SETTINGS_KEY_IPCSOCKETPATH "ipcsocketpath"
#define SETTINGS_KEY_PDFVIEWER "pdfviewer"
#define SETTINGS_KEY_EXCLUDEDPATHS "excludedpaths"
#define SETTINGS_KEY_MOUNTPATHS "mountpaths"
#define SETTINGS_KEY_PREVIEWSPERPAGE "previewsPerPage"
#define SETTINGS_KEY_SEARCHHISTORY "searchhistory"
#define SETTINGS_KEY_PREVIEWS_SCROLL_HORIZONTALLY "horizontalscroll"

namespace Common
{
void setupAppInfo();
QString databasePath();
QString ipcSocketPath();
void setPdfViewer();
bool initSqliteDatabase(QString path);
void ensureConfigured();
QStringList excludedPaths();
QStringList mountPaths();
bool isTextFile(QFileInfo fileInfo);
bool isMountPath(QString path);
bool noSandboxModeRequested();
QString versionText();
QString userManualUrl();
} // namespace Common
#endif
