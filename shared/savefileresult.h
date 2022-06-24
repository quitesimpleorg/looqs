#ifndef SAVEFILERESULT_H
#define SAVEFILERESULT_H

#include <QStringList>

enum SaveFileResult
{
	OK,
	SKIPPED,
	DBFAIL,
	PROCESSFAIL,
	NOTFOUND,
	NOACCESS
};

static inline QString SaveFileResultToString(SaveFileResult sfr)
{
	QStringList SaveFileResultStr;

	SaveFileResultStr << "OK"
					  << "SKIPPED"
					  << "DBFAIL"
					  << "PROCESSFAIL"
					  << "NOTFOUND"
					  << "NOACCESS";

	return SaveFileResultStr[(int)sfr];
}

#endif // SAVEFILERESULT_H
