#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H
#include "filedata.h"

class SearchResult
{
  public:
	FileData fileData;
	QVector<unsigned int> pages;
	bool wasContentSearch = false;
};

#endif // SEARCHRESULT_H
