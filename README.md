# looqs - FTS for the Linux desktop with previews for search results
looqs creates a full text search index for your files. It allows you to look at previews where your
search terms have been found, as shown in the screenshots below.


## Screenshots
The screenshots in this section may occasionally be slightly outdated, but they are usually recent enough to get an overall impression of the current state.

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
 * **No daemons**. As some other desktop search projects are prone to have annoying daemons running that eat system resources away, this solution should make do without daemons where possible.
 * **Easy setup**. Similiarly, there should be no need for heavy-weight databases. Instead, this solution tries to squeeze out the most from simple approaches. In particular, it relies on sqlite.
 * **GUI & CLI**. Provide CLI interfaces and GUI interfaces
 * **Sandboxing**. As reading and rendering lots of formats naturally opens the door for security bugs, those tasks are offloaded to small, sandboxed sub-processes to mitigate the effect of exploited vulnerabilities.

## Supported platforms
Linux (on amd64) is currently the main focus. Currently, I don't plan on supporting anything else and the sandboxing architecture does not make it likely. I suppose a version without sandboxing might be conceivable for other platforms, but I have no plans or resources to actively target anything but Linux at this point.

### Licence
GPLv3. 

### Contributing
Fow now, github issues and pull-requests are preferred, but you can also just email
your patches or issues to : looqs at quitesimple.org


## Build

### Ubuntu 21.10/22.04
```
git submodule init
git submodule update
sudo apt install build-essential qtbase5-dev libpoppler-qt5-dev libuchardet-dev libquazip5-dev
qmake
make
```



## Documentation
Please see [Usage.md](USAGE.md) for the user manual. 

## Packages
Coming soon™
