#ifndef FILESAVEROPTIONS_H
#define FILESAVEROPTIONS_H

class FileSaverOptions
{
  public:
	bool verbose = false;
	bool keepGoing = false;
	bool metadataOnly = false;
	/* Whether those previously explicitly without content should be filled */
	bool fillExistingContentless = false;
};

#endif // FILESAVEROPTIONS_H
