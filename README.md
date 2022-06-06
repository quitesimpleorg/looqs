# looqs - Full-text search with previews for your files
looqs is a tool that creates a full-text search index for your files. It allows you to look at previews where your
search terms have been found, as shown in the screenshots below.

## Screenshots
The screenshots in this section may occasionally be slightly outdated, but they are usually recent enough to get an overall impression of the current state of the GUI.

### Preview
looqs allow you to look inside files. It marks what you have searched for. 

![Screenshot looqs](https://garage.quitesimple.org/assets/looqs/orwell.png)
![Screenshot looqs search fstream](https://garage.quitesimple.org/assets/looqs/fstream_write.png)

### Results list
#### Classic results list
Just enter what you want to find, it will search paths and file content. 
![Screenshot looqs results](https://garage.quitesimple.org/assets/looqs/looqs_diary.png)

#### Searching with filters
You can be more specific to get what you want with filters

**Filters (long form)**
![Screenshot looqs results](https://garage.quitesimple.org/assets/looqs/opearting_systems_looqs.png)

**Filters (short form)**

There is no need to write the long form of filters. There are also booleans available

![Screenshot looqs results](https://garage.quitesimple.org/assets/looqs/looqs_beatles_marley.png)


## Current status
Last version: 2022-0X-XX, v0.1

Please see [Changelog](CHANGELOG.md) for a human readable list of changes.


## Goals and principles
 * **Find & Preview**. Instead of merely telling you where your search phrase has been found, it should also render the corresponding portion/pages of the documents and highlight the searched words.
 * **No daemons**. As some other desktop search projects are prone to have annoying daemons running that eat system resources away, this solution should make do without daemons where possible.
 * **Easy setup**. Similarly, there should be no need for heavy-weight databases. Instead, looqs tries to squeeze out the most from simple approaches. In particular, it relies on sqlite.
 * **GUI & CLI**. Provide CLI interfaces and GUI interfaces
 * **Sandboxing**. As reading and rendering lots of formats naturally opens the door for security bugs, those tasks are offloaded to small, sandboxed sub-processes to mitigate the effect of exploited vulnerabilities.

 
## Features 
- GUI, CLI interface
- Indexing of file path and some metadata.
- Indexing of file file content for FTS search. Currently: .pdf, odt, docx, plaintext.
- Preview of file formats: Currently: .pdf, plaintext
- Highlight searched terms.
- Quickly open PDF viewer or text editor at location of preview
- Search filters

## Supported platforms
Linux (on amd64) is currently the main focus. Currently, I don't plan on supporting anything else and the sandboxing architecture does not make it likely. I suppose a version without sandboxing might be conceivable for other platforms, but I have no plans or resources to actively target anything but Linux at this point.

### Licence
GPLv3.

### Contributing
Please see the [Contribution guidelines](CONTRIBUTING.md) file. 

## Documentation
Please see [USAGE.md](USAGE.md) for the user manual. There is also [HACKING.md](HACKING.md) with more technical information.


## Build

### Ubuntu 21.10/22.04

To build on Ubuntu, clone the repo and then run: 
```
git submodule init
git submodule update
sudo apt install build-essential qtbase5-dev libpoppler-qt5-dev libuchardet-dev libquazip5-dev
qmake
make
```

The GUI is located in `gui/looqs-gui`, the binary for the CLI is in `cli/looqs`

This may also work on Debian, but it's untested.


## Packages
Coming soon™
