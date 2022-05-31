# looqs - User guide
This document is still work in progress.

# General points
Please consult the [README](README.md) for a description of what looqs is and on how to obtain it.

# Current Limitations and things to know
You should be aware of the following:

- It may seem naturally, but the GUI and CLI operate on the same database, so if you add files using the CLI, the GUI will search them too.

- If a file is listed in the "Search results" tab, it does not imply that a preview will be available in the "Previews" tab, as looqs can search more file formats than it can generate previews for currently. 

- Database paths are stored inefficiently, not deduplicated to simplify queries. This may add up quickly. Also, each PDF text is stored twice. Each page separately + the whole document to simplify queries.

At the time this section was written, 167874 files were in my index. A FTS index was built for 14280 of those, of which 4146 were PDF documents. The PDFs take around 10GB storage space on the filesystem. All files for which an FTS has been built are around 7GB in size on the filesystem.

The looqs database has a size of 1.6 GB.


## Config
It's in `$HOME/.config/quitesimple.org/looqs.conf`. It will be created on first execution of the CLI or GUI
interface.

Database default path: `$HOME/.local/share/quitesimple.org/looqs/looqs.sqlite`. If this does not work for
you, move it and adjust adjust the path in the config file.


## GUI
It's minimal at this point, therefore some settings must be performed by editing the config file.

### First run
You will be presented with an empty list. Go to the "Index" tab, add some directories and click "Start indexing".

For large directories the progress bar is essentially just decoration. As long as you see the counters
increase, everything is fine even if it seems the progress bar is stuck.

The indexing can be stopped. If you run it again you do not start from scratch, because looqs knows
which files have been modified or not since they have been added to the index. Thus, files will
only be reprocedded when necessary.

### Configuring PDF viewer
It's most convenient if, when you click on a preview, the PDF reader opens the page you clicked. For that, looqs needs to know which viewer you want to launch.

It tries to auto detect some common viewers. You must set the value yourself if it doesn't do something you
like, such as not opening your favorite viewer. In the command line options, "%f" represents the filepath, "%p" the page number.

### Preview tab
The preview tab shows previews. It marks you search keywords too. Click on a preview to open the file.
You can click right to copy the file path, or open the containing folder. Hovering tells you which file
the preview originates from.

## CLI
The CLI command "looqs" has helptext provided. This documentation is incomplete at the moment.

### First run
There is no point in using the "search" command on the first run. Add some files.


### Adding files
To add files to the index, run ``looqs add [path]```, where 'path' can be a directory or a single file.
If the path is a directory, the directory will be recursively descended, and all files in there added.

"Skipped" implies the file has not been changed since it has been added to the index. If it changed,
the index content will be updated. 

### Searching files
Of course the CLI will not render any previews, but it can show you the paths where search results
have been found.

```
looqs search [terms...]
```

There is an implicit "AND" condition, meaning if you search for "photo" and "mountain", only paths
will be shown containing both terms, but not either alone.

### Deletion and Fixing Out of sync index
You sometimes delete files, to get rid of those from the index too:

```
looqs delete --deleted --dry-run
```

This commands lists all files which are indexed, but which cannot be found anymore.

Remove them using:
```
looqs delete --deleted --verbose
```

You can also delete by pattern:

```
looqs delete --pattern '*.java'
```

Delete never removes anything from the file system, it only operates on the database.

### Updating files
The content and metadata index for files can be updated:

```
looqs update -n
```

Those files still exist, but the content that has been indexed it out of date. This can be corrected with

```
looqs update
```

This will not add new files, you must run `looqs add` for this. For this reason, most users
will probably seldomly use the 'update' command alone.



