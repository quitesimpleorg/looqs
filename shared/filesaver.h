#ifndef FILESAVER_H
#define FILESAVER_H
#include <QSqlDatabase>
#include <QFileInfo>
#include "filesaveroptions.h"
#include "pagedata.h"
#include "filedata.h"
#include "sqlitedbservice.h"
#include "common.h"
class FileSaver
{
  private:
	SqliteDbService *dbService;
	QStringList excludedPaths = Common::excludedPaths();
	FileSaverOptions fileSaverOptions;

  public:
	FileSaver(SqliteDbService &dbService);
	SaveFileResult addFile(QString path);
	SaveFileResult updateFile(QString path);
	SaveFileResult saveFile(const QFileInfo &fileInfo);
	int processFiles(const QVector<QString> paths, std::function<SaveFileResult(QString path)> saverFunc);
	int addFiles(const QVector<QString> paths);
	int updateFiles(const QVector<QString> paths);

	void setFileSaverOptions(FileSaverOptions options)
	{
		this->fileSaverOptions = options;
	}
};

#endif // FILESAVER_H
