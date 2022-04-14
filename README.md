# looqs - Looks for files. And looks inside them
looqs creates a full text search for your files. It allows you to look at previews where your
search terms have been found. 

Currently, this allows you search all indexed pdfs and take a look at the pages side by side in an instant, as shown in the screenshots.

## Screenshots
### List
![Screenshot looqs results](https://garage.quitesimple.org/assets/looqs/opearting_systems_looqs.png)

### Preview
![Screenshot looqs](https://garage.quitesimple.org/assets/looqs/orwell.png)
![Screenshot looqs search fstream](https://garage.quitesimple.org/assets/looqs/fstream_write.png)


## Current status
Last version: 2022-0X-XX, v0.1

Please see [Changelog](CHANGELOG.md) for a human readable list of changes. 


## Goals and principles
 * **Find & Preview**. Instead of merely telling you where your search phrase has been found, it should also render the corresponding portion/pages of the documents and highlight the searched words.
 * **No daemons**. As other solutions are prone to have annoying daemons running that eat system resources away, this solution should make do without daemons if possible.
 * **Easy setup**. Similiarly, there should be no need for heavy-weight databases. Instead, this solution tries to squeeze out the most from simple approaches. In particular, it relies on sqlite.
 * **GUI & CLI**. Provide CLI interfaces and GUI interfaces
 * **Sandboxing**. As reading and rendering lots of formats naturally opens the door for security bugs, those tasks are offloaded to small, sandboxed sub-processes to mitigate the effect of exploited vulnerabilities.

## Supported platforms
Linux on amd64 is currently the main focus. 

## Build
### Ubuntu 21.04
```
sudo apt install build-essential qtbase5-dev libpoppler-qt5-dev libuchardet-dev libquazip5-dev
qmake
make
```

## Documentation
Coming soon™

## Packages
Coming soon™
