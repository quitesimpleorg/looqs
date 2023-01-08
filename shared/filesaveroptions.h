#ifndef FILESAVEROPTIONS_H
#define FILESAVEROPTIONS_H

class FileSaverOptions
{
  public:
	bool verbose = false;
	bool keepGoing = false;
	bool pathsOnly = false;
	/* Whether those previously explicitly without content should be filled */
	bool fillPathsOnlyWithContent = false;
};

#endif // FILESAVEROPTIONS_H
