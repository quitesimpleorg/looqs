# looqs - Hacking

## Introduction
If you are interested on how to contribute, please see the file [CONTRIBUTING.md](CONTRIBUTING.md) which contains the instructions on how to submit patches etc.

## Security
The architecture ensures that the parsing of documents and the preview generation is sandboxed by [exile.h](https://github.com/quitesimpleorg/exile.h). looqs uses a multi-process architecture to achieve this.

Qt code is considered trusted in this model. While one may critize this, it was the only practical solution. looqs uses Qt's serialization mechanism and other classes to communicate between the non-sandboxed GUI process and the sandboxed processes.

Set the enviornment variable `LOOQS_DISABLE_SANDBOX=1` to disable sandboxing. It's intended for troublehshooting.

## Database
The heart is sqlite, with the FTS5 extensions behind the full-text search. While FTS may not be sqlite's strong suit, I definitely did not want to run one of those oftenly recommended heavy (Java based) solutions. I explored other options like Postgresql, I've discard them due to some limitations back then. It's also natural to use sqlite as it's
used for metadata in general.

Down the road, alternatives will be explored of course if sqlite should not suffice anymore.

## File format support
The pdf library is libpoppler. Files such as .odt or .docx documents are opened with libquazip. The XML files in there are not parsed,
looqs simply strips the tags and that seems to work fine so far. Naturally, this is not the "proper way", so there is room for improvement maybe here. However, those file formats are not a huge priortiy for me personally. libuchardet does encoding detection and conversion.

Naturally, looqs won't be able to index and render previews for everything. Such approach would create a huge bloated binary. In the future, there will be some plugin system of some sorts, either we will load .so objects or use subprocesses.

## Name
looqs looks for files. You as the user can also look inside them. The 'k' in "looks" was replaced by a 'q'. Originally, I wanted my projects to have "qs" (for quitesimple) in their name. While that quirk is abandoned now, this got us to looqs.



