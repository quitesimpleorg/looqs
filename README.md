# looqs - Full-text search with previews for your files
looqs is a tool that creates a full-text search index for your files. It allows you to look at previews where your search terms have been found, as shown in the screenshots below.

## Screenshots
### Preview
looqs allows you to look inside files. It highlights what you have searched for.

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

The screenshots in this section may occasionally be slightly outdated, but they are usually recent enough to get an overall impression of the current state of the GUI.

## Current status
Latest version: 2022-09-10, v0.7

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

### Debian/Ubuntu

To build on Ubuntu and Debian, clone the repo and then run:
```
git submodule init
git submodule update
sudo apt install build-essential qtbase5-dev libpoppler-qt5-dev libuchardet-dev libquazip5-dev
qmake
make
```

### Void
```
# as root
xbps-install qt5-devel poppler-qt5-devel quazip-qt5-devel uchardet-devel gcc make
# as user
git submodule init
git submodule update
qmake
make
```

The GUI is located in `gui/looqs-gui`, the binary for the CLI is in `cli/looqs`

## Packages
At this point, looqs is not in any official distro package repo, but I maintain some packages.

### Ubuntu 22.04
Latest release can be installed using apt from the repo.
```
# First, obtain key, assume it's trusted.
wget -O- https://repo.quitesimple.org/repo.quitesimple.org.asc  | gpg --dearmor > repo.quitesimple.org-keyring.gpg
cat repo.quitesimple.org-keyring.gpg | sudo tee -a /usr/share/keyrings/repo.quitesimple.org.gpg > /dev/null

echo "deb [arch=amd64 signed-by=/usr/share/keyrings/repo.quitesimple.org.gpg] https://repo.quitesimple.org/debian/ $(lsb_release -sc) main" | sudo tee /etc/apt/sources.list.d/quitesimple.list
sudo apt-get update
sudo apt-get install looqs
```

### Prebuilt tarball (distro-agnostic) (EXPERIMENTAL)
looqs is also distributed as a tarball containing prebuilt binaries and its library dependencies. The tarball is
built using the Gentoo Hardened toolchain and the Qt version is smaller than what distributions usually
include. It does not include libraries that should mess with fontrendering or the graphics stack. The binaries should run on any recent Linux distribution (requires glibc 2.34 or newer at least) and expects
dependencies such as libGL to be provided by your distribution already (should be the case).

It's considered experimental for two reasons. Firstly, looqs has no updater (yet). You will have to manually check for updates. Secondly, I can't guarantee that I'll be quick with updates of the tarball specifically if the library versions become outdated between looqs updates.

You are therefore encouraged to use distro-native packages or to build it yourself if possible.

The tarball can be obtained here: https://repo.quitesimple.org/tarball/looqs

Quick start:

```
# Verify sig, see the end of this document: gpg --verify looqs-v0.4.tar.xz.sig
tar xf looqs-v0.4.tar.xz # Replace with the actual version you have obtained
cd looqs-v0.4
./looqs-gui # or ./looqs for the CLI
```

An AppImage may accompany the tarball in the future.


### Other distros
I'll probably add a package for voidlinux at some point and maybe will provide a Gentoo ebuild. However, I would appreciate help for others distros. If you create a package, let me know!


### Signature verification
Release tags can be verified with  [my PGP public key](https://quitesimple.org/share/pubkey). For what little it's worth, its fingerprint is: `C342 CA02 D2EC 2E14 F3C3  D5FF 7F7B 4C08 02CD 02F2`

Packages can be verified with the [repo-specific public key](https://repo.quitesimple.org/repo.quitesimple.org.asc). For what little it's worth, its fingerprint is: `1B49 45B3 16B2 468A 3DAC  C1E0 75EF 3FE8 D753 C8F9`
