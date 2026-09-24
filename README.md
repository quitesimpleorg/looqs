# looqs - Full-text search with previews for your files
looqs is a tool that creates a full-text search index for your files. It allows you to look at previews where your search terms have been found, as shown in the screenshots below.

## Screenshots
### Preview
looqs allows you to look inside files. It highlights what you have searched for.

![Screenshot looqs](https://quitesimple.org/share/assets/looqs/orwell.png)
![Screenshot looqs search fstream](https://quitesimple.org/share/assets/looqs/fstream_write.png)

### Results list
#### Classic results list
Just enter what you want to find, it will search paths and file content.
![Screenshot looqs results](https://quitesimple.org/share/assets/looqs/looqs_diary.png)

#### Searching with filters
You can be more specific to get what you want with filters

**Filters (long form)**
![Screenshot looqs results](https://quitesimple.org/share/assets/looqs/opearting_systems_looqs.png)

**Filters (short form)**

There is no need to write the long form of filters. There are also booleans available

![Screenshot looqs results](https://quitesimple.org/share/assets/looqs/looqs_beatles_marley.png)

The screenshots in this section may occasionally be slightly outdated, but they are usually recent enough to get an overall impression of the current state of the GUI.

## Current status
Latest version: 2024-07-21, v0.10

As my personal workflow has changed, the intrinsic motivation to work on this project has gone down. Development is therefore paused.

Please keep in mind: looqs is still at an early stage and may exhibit some weirdness and contain bugs.

Please see [Changelog](CHANGELOG.md) for a human readable list of changes. For download instructions, see
further down this document.


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
- Preview of file formats: Currently: .pdf, .odt, plaintext
- Highlight searched terms.
- Quickly open PDF viewer or text editor at location of preview
- Search filters

## Supported platforms
Linux (on amd64) is currently the main focus. Currently, I don't plan on supporting anything else and the sandboxing architecture does not make it likely. I suppose a version without sandboxing might be conceivable for other platforms, but I have no plans or resources to actively target anything but Linux at this point.

### Licence
GPLv3.

For the dependencies/third-party libraries, see: LICENSE-3RD-PARTY

### Contributing
Please see the [Contribution guidelines](CONTRIBUTING.md) file.

## Documentation
Please see [USAGE.md](USAGE.md) for the user manual. There is also [HACKING.md](HACKING.md) with more technical information.

## Build

### Signature verification
Release tags can be verified with  [my PGP public key](https://quitesimple.org/share/pubkey). For what it's worth, its fingerprint is: `C342 CA02 D2EC 2E14 F3C3  D5FF 7F7B 4C08 02CD 02F2`

### Debian/Ubuntu

To build on Ubuntu and Debian, clone the repo and then run:
```
git submodule init
git submodule update
sudo apt install build-essential qt6-base-dev libqt6sql6-sqlite libpoppler-qt6-dev libuchardet-dev libquazip1-qt6-dev
qmake6
make
```

The GUI is located in `gui/looqs-gui`, the binary for the CLI is in `cli/looqs`


