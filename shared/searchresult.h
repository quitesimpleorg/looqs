#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H
#include "filedata.h"

class SearchResult
{
  public:
	FileData fileData;
	unsigned int page;
	bool wasContentSearch = false;
};

#endif // SEARCHRESULT_H
