#ifndef COMMON_H
#define COMMON_H
#include <QCoreApplication>

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
} // namespace Common
#endif
