#ifndef COMMON_H
#define COMMON_H
#include <QCoreApplication>

namespace Common
{
void setupAppInfo();
QString databasePath();
QString ipcSocketPath();
bool initSqliteDatabase(QString path);
void ensureConfigured();
} // namespace Common
#endif
