#ifndef SAVEFILERESULT_H
#define SAVEFILERESULT_H

#include <QStringList>

enum SaveFileResult
{
	OK,
	OK_WASEMPTY, /* The file was successfully read but no content could be extracted. Might be just an empty document or
					so*/
	SKIPPED,	 /* The file was not processed/saved, because it has not changed */
	DBFAIL,		 /* A database error occured */
	PROCESSFAIL, /* General processor failure */
	NOTFOUND,	 /* The file was not found */
	NOACCESS	 /* The process can't read the file */
};

static inline QString SaveFileResultToString(SaveFileResult sfr)
{
	QStringList SaveFileResultStr;

	SaveFileResultStr << "OK"
					  << "OK_WASEMPTY"
					  << "SKIPPED"
					  << "DBFAIL"
					  << "PROCESSFAIL"
					  << "NOTFOUND"
					  << "NOACCESS";

	return SaveFileResultStr[(int)sfr];
}

static inline bool isErrorSaveFileResult(SaveFileResult result)
{
	return result == DBFAIL || result == PROCESSFAIL || result == NOTFOUND || result == NOACCESS;
}

#endif // SAVEFILERESULT_H
