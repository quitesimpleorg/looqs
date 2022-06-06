#ifndef COMMON_H
#define COMMON_H
#include <QCoreApplication>
#include <QFileInfo>
namespace Common
{
void setupAppInfo();
QString databasePath();
QString ipcSocketPath();
void setPdfViewer();
QString findInPath(QString needle);
bool initSqliteDatabase(QString path);
void ensureConfigured();
QStringList excludedPaths();
QStringList mountPaths();
bool isTextFile(QFileInfo fileInfo);
bool isMountPath(QString path);
bool noSandboxModeRequested();
QString versionText();
} // namespace Common
#endif
