# looqs - User guide
This document is still work in progress.

## General points
Please consult the [README](README.md) for a description of what looqs is and on how to obtain it.

looqs is still at an early stage and may exhibit some weirdness and contain bugs.

## Current Limitations and things to know
You should be aware of the following:

- Lags are to be expected for networked mount points such as SMB and NFS etc.

- It may seem natural, but the GUI and CLI operate on the same database, so if you add files using the CLI, the GUI will know about them too.

- If a file is listed in the "Search results" tab, it does not imply that a preview will be available in the "Previews" tab, as looqs can search more file formats than it can generate previews for currently.

- Existing files are considered modified when the modification time has changed. looqs currently does not check whether the content has changed.

## Config
The config file is in `$HOME/.config/quitesimple.org/looqs.conf`. It will be created on first execution of the CLI or GUI interface. Generally, you should not edit this file directly. Instead, use the "Settings" tab in the GUI.

Database default path: `$HOME/.local/share/quitesimple.org/looqs/looqs.sqlite`. If you prefer a different path, move it and adjust the path.


## GUI

### First run
You will be presented with an empty list. Go to the **"Index"** tab, add some directories and click **"Start indexing"**.

### Indexing
For large directories the progress bar is essentially just decoration. As long as you see the counters
increase, everything is fine even if it seems the progress bar is stuck.

The indexing can be stopped. If you run it again you do not start from scratch, because looqs knows
which files have been modified since they have been added to the index. Thus, files will
only be reprocessed when necessary. Note that cancellation itself may take a moment as files finish processing.

The counters increase in batches, therefore it's normal that it seems no progress is being made, particularly when processing lots of large documents. This aspect will be improved in a future version.

### Search
The text field at the top is where you type your query. It can be selected quickly using **CTRL + L**. Filters are available, see this document at the end. By default, both the full path and the content are searched. Path names take precedence, i. e. they will appear the top of the list.

**CTRL + F**: This is helpful shortcut if you want to perform several searches. Consider the following
query: "p:(docs) c:(invoice credit card)". Press CTRL+F to highlight 'invoice credit card'. This way
you can quickly perform content searches in paths containing 'docs'.

**CTRL + W**: Removes the last filter. If we take above's example "p:(docs) c:(invoice credit card)" again, then CTRL + W kills "c:(invoice credit card)".

The arrow keys (up and down) can be used to go back and forward in the search history.

### Configuring PDF viewer
It's most convenient if, when you click on a preview, the PDF reader opens the page you clicked. For that, looqs needs to know which viewer you want to launch.

It tries to auto detect some common viewers. You must set the value in the "Settings" tab yourself if the
default does not work for you. In the command line options, "%f" represents the filepath, "%p" the page number.

### Previews tab
The 'previews' tab shows previews. It marks your search keywords too. Click on a preview to open the file.
A right click on a preview allows you to copy the file path, or to open the containing folder.

When the combobox is set to "All previews", the previews are ordered by relevance from all documents/pages.

By default, a vertical scrolling is active. In the settings, it can be changed to horizontal scroll, which may be
preferred by users of (larger) wide screen monitors.

### Syncing index
Over time, files get deleted or their content changes. Go to **looqs** -> **Sync index**. looqs will reindex the content of files which have been changed. Files that cannot be found anymore will be removed from the index.

Reindexing a path using the "Index" tab will index new files and update existing ones. Currently however, this does not deal with deleted files.

I recommend doing a sync from time to time.

## CLI
The CLI command "looqs" comes with helptext. This documentation is incomplete at the moment.

### First run
There is no point in using the "search" command on the first run. Add some files if not done so already.

### Adding files
To add files to the index, run ```looqs add [path]```, where 'path' can be a directory or a single file.
If the path is a directory, the directory will be recursively descended, and all files in there added.

"Skipped" implies the file has not been changed since it has been added to the index. If it has changed, the index content will be updated.

### Searching files
Of course the CLI will not render any previews, but it can show you the paths where search results
have been found.

```
looqs search [terms...]
```

There is an implicit "AND" condition, meaning if you search for "photo" and "mountain", only paths
will be shown containing both terms, but not either alone.

### Deletion and Fixing Out of sync index
To get rid of deleted files from the index, run:

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

The equivalent of the GUI sync command is:
```
looks update -v --continue --delete
```


### Updating files
The content and metadata index for files can be updated:

```
looqs update -n
```

Those files still exist, but the content that has been indexed it out of date. This can be corrected with

```
looqs update
```

This will not add new files, you must run ```looqs add``` for this. For this reason, most users
will probably seldom use the 'update' command alone.


## Tips

### Keeping index up to date
The most obvious way is to use the GUI to add your favorite paths in the "Index" tab. Then occasionally, just rescan. This works for me personally, looqs quickly picks up new files. This however may not be good enough for some users.

Some users may prefer setting up cronjobs or wire up the CLI interface with file system monitoring tools such as [adhocify](https://github.com/quitesimpleorg/adhocify).

### lh shell alias
If you are in a shell and you know your file is somewhere in your current directory or its subdirs, and those
are indexed by looqs, you may find the lh (look here) alias useful:

```
alias lh='looqs search $(pwd)'
```

So typing "lh recipes" searches the current dir and its subdirs for a file containing 'recipes'. Alternatively, a "lh c:(rice)" may be a quick grep alternative.

## Query syntax / Search filters
A number of search filters are available.

| Filter (long) |  Filter (short) | Explanation |
| ----------- | ----------- |----------- |
| path.contains:(term) | p:(term) | Pretty much a SQL LIKE '%term%' condition, just searches the path string |
| path.ends:(term) | pe:(term) | Filters path ending with the specified term, e. g.: pe:(.ogg) |
| path.begins:(term) | pb:(term) |  Filters path beginning with the specified term |
| contains:(terms) | c:(terms) | Full-text search, also understands quotes |
| limit:(integer) | - | Limits the number of results. The default is 1000. Say "limit:0" to see all results |
| tag:(tagname) | t:(tagname) | Filter for files that have been tagged with the corresponding tag |

Filters can be combined. The booleans AND and OR are supported. Negations can be applied too, except for c:(). Negations are specified with "!".
The AND boolean is implicit and thus entering it strictly optional.

Examples:

| Query | Explanation |
| ----------- | ----------- |
|pe:(.ogg) p:(marley)| Finds paths that end with .ogg and contain 'marley' (case-insensitive)
|p:(slides) support vector machine &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; |Performs a content search for 'support vector machine' in all paths containing 'slides'|
|p:(notes) (pe:(odt) OR pe:(docx)) &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;|Finds files such as notes.docx, notes.odt but also any .docs and .odt when the path contains the string 'notes'|
|memcpy !(pe:(.c) OR pe:(.cpp))| Performs a FTS search for 'memcpy' but excludes .cpp and .c files.|
|c:("I think, therefore")|Performs a FTS search for the phrase "I think, therefore".|
|c:("invoice") Downloads|Equivalent to c:("invoice") p:("Downloads")|
|p:(Downloads) invoice|Equivalent to c:("invoice") p:("Downloads")|
