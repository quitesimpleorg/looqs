#ifndef INDEXSYNCER_H
#define INDEXSYNCER_H
#include "sqlitedbservice.h"
#include "filesaveroptions.h"
class IndexSyncer : public QObject
{
	Q_OBJECT
  private:
	SqliteDbService *dbService = nullptr;
	FileSaverOptions fileSaverOptions;
	bool removeDeletedFromIndex = true;
	bool dryRun = false;
	QString pattern;

	std::atomic<bool> stopToken{false};

  public:
	IndexSyncer(SqliteDbService &dbService);

	void setFileSaverOptions(FileSaverOptions options);

  public slots:
	void sync();
	void cancel();
	void setDryRun(bool dryRun);
	void setRemoveDeletedFromIndex(bool removeDeletedFromIndex);
	void setPattern(QString pattern);

  signals:
	void error(QString error);
	void removed(QString path);
	void removedDryRun(QString path);
	void updatedDryRun(QString path);
	void updated(QString path);
	void finished(unsigned int totalUpdated, unsigned int totalDeleted, unsigned int totalErrored);
};

#endif // INDEXSYNCER_H
