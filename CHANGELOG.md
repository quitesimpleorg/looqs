# looqs: Release notes
## 2024-05-xx - v0.10
CHANGES:
 - Move to Qt6, drop Qt5 support.
 - Index ToC of PDFs
 - ToC can be searched with filter: toc:(query). (Basic, more to come).
 - sqlite: Improvements so rare "database locked" errors don't occur.
 - Minor fixes
 - Add packages: Ubuntu 23.10.
 - Remove packages: Ubuntu 22.10

## 2023-05-07 - v0.9
Highlights: Tag support. Also begin new index mode to only index metadata (currently only path + file size, more to come).

Note: Upgrading can take some time as new column indexes will be added

CHANGES:

 - gui: Improve font rendering in previews
 - gui: Allow indexing only metadata
 - gui: Allow adding content for files which only had metadata indexed before
 - gui: Allow assigning tags by right clicking on paths
 - cli: "add" command: Implement --verbose (-v)
 - cli: "add" command: Implement --no-content and --fill-content
 - cli: Add "tag" command which allows managing tags for paths.
 - search: Add "tag:()", "t:()" filters
 - Minor improvements and refactorings under the hood
 - Add packages: Ubuntu 23.04.


## 2022-11-19 - v0.8.1

CHANGES:
 - Fix regression causing previews in second (and higher) result page to not render
 - Minor improvements

## 2022-10-22 - v0.8

CHANGES:
 - For new, not previously indexed files, start creating an additional index using sqlite's experimental trigram tokenizer. Thanks to that, we can now match substrings >= 3 of an unicode sequence. Results of the usual index are prioritized.
 - GUI: Ensure order of previews matches ranking exactly. Previously, it depended simply on the time preview generators took, i. e. it was more or less a race.
 - Report progress more often during indexing, so users don't get the impression that it's stuck when processing dirs with large documents.
 - Fix a regression that caused phrase queries to be broken
 - Minor improvements
 - Add packages: Ubuntu 22.10.

## 2022-09-10 - v0.7

CHANGES:

 - GUI: Add vertical scroll option, default to it. Most feedback considered horizontal scrolling unnatural.
 - GUI: Previews: Improve plaintext preview snippet selection by prioritizing those which contain the most search terms
 - GUI: Previews: Only highlight whole words, not parts in words.
 - GUI: Previews: Don't treat text wrapped inside '<' '>' in plaintext files as HTMl tags which caused such text to get lost in previews.
 - GUI: Avoid triggering preview generation in some cases even when previews tab is not the active one
 - GUI: Implement a search history. Allow going up/down with arrow keys in the search field.
 - GUI: Previews: Allow CTRL + mouse wheel to zoom in on previews
 - General: Fix an incorrect sqlite query which caused the ranking information of search results to be lost. "All files" filter in the previews tab therefore now orders by the seemingly most relevant pages, across all documents.
 - General: Fix handling of content search queries with prefix terms (those ending in '*').
 - GUI: Show how many results are previewable.
 - GUI: Refactor to improve stability of sandboxed IPC worker in order to avoid rare segfaults.

## 2022-08-14 - v0.6
This release features multiple fixes and enhancements.

Bad news first: It drops a trivial trigger that appeared to work quite fine, but silently may cause "unpredictability" of the sqlite FTS5 index ( [9422a5b494](https://github.com/quitesimpleorg/looqs/commit/9422a5b494dabd0f1324dc2f92a34c3036137414) ). As a result, FTS queries may return weird and unexplainable results. This is not reasonably automatically recoverable by looqs. I strongly recommend creating a clean, new database. All previous versions are affected. To do that, go to "Settings" -> checking "Remove old database on save" -> "Save settings and restart". Alternatively, specify a new path to keep the old database.

CHANGES:

 - GUI: Add line numbers and context lines to plaintext previews
 - GUI: Fix case where previews for old queries would have still been visible if new query would not create previews
 - GUI: Add CTRL + F, CTRL+W, CTRL+Tab, CTRL+Shift+Tab shortcuts (see user manual)
 - GUI: Add checkbox in "Settings" tab allowing to delete database.
 - General: Fix wrong regexes that caused query errors with chars like -
 - General: Drop trigger sending incomplete sqlite fts5 deletion command, causing undefined index behaviour

## 2022-07-30 - v0.5.1

CHANGES:

 - gui: Fix regression in implicit paths queries introduced in previous version

## 2022-07-29 - v0.5
This release features multiple fixes and enhancements.

It changes the database to drop an unused content column. Dropping it allows us
to change to a contentless sqlite FTS index which frees up space.

Upgrading might take a few seconds to a few minutes as looqs will recreate the whole index.
How long this will take depends on the size of your database.

The other major highlight is preview support for .odt files (unformatted, like plaintext).

List of changes:

 - General: As Ubuntu 21.10 is EOL, no looqs package will be provided for it any longer
 - General: Update database scheme to drop unused content column and free up space
 - General: Properly escape FTS arguments passed to sqlite to avoid query errors on some terms
 - GUI: Fix double searches and results when explicit content search filters are provided
 - GUI: Previews: Plaintext previews now obey scale selection too
 - GUI: Previews: Begin basic, unformatted previews of .odt files
 - GUI: Previews: Add file path below every preview for convenience (hovering unnecessary now).
 - GUI: Previews: Fix bug causing an empty preview if a plaintext file started with a searched word
 - GUI: Add menu option to open user manual
 - GUI: Show progress dialog during database upgrade
 - General: Correct error count in some conditions for failed paths
 - General: Update user manual
 - Minor improvements

## 2022-06-29 - v0.4
This release makes several minor improvements and begins prebuilt binaries of looqs that (should) run
on any recent Linux distribution.

 - General: Begin new, experimental distro-agnostic tarball containing prebuilt binaries and libs. See the README for more information.
 - GUI: In the "Previews" tab, allow filtering by file, to only show previews for a specific file.
 - GUI: Add "Show previews" context menu option to files in result (if available)
 - General: Fix build with libquazip 1.X
 - General: Properly report access errors as a failure during indexing
 - GUI: Add button to export a list of all paths that failed
 - General: Improve dir scan threading
 - CLI: Improve helptext
 - General: Add voidlinux build instructions

## 2022-06-14 - v0.3
CHANGES:
- GUI: Add settings tab to configure various settings
- GUI: Don't render previews for results that do not originate from a content search. This was confusing.
- GUI: Remove 'open config file' menu action
- GUI: Highlight "words" that are actually numbers in preview texts
- search: Search for "words" that are a number, e. g. 23. Previously, those were ignored due to a regression.
- CLI: Minor cleanups.
- sandbox: Improve preview generation sandbox further by restricting it more
- sandbox: Fix case where activiation would fail during indexing in some cases on kernels without landlock.
- Remove 'firstrun' setting


## 2022-06-07 - v0.2
CHANGES:
- Sandboxing: Add environment variable `LOOQS_DISABLE_SANDBOXING` to disable sandboxing. This is intended for troubleshooting
- Sandboxing: Fix issue where activation failed on kernels without landlock

## 2022-06-06 - v0.1
The first release comes with basic functionality. It's a start that can be considered useful to some degree.

looqs is still at an early stage and may exhibit some weirdness and contain bugs.

Tested architectures: amd64.

CHANGES:
- CLI command "looqs" to add/update/delete and search
- GUI: "looqs-gui" to search, render previews, and add files to index
- General: Add multi-threaded indexing of all files (paths, mtime)
- General: Generate sqlite based full-text search index for: .pdf,.odt,.ods, text files
- General: Sandboxed content processing
- GUI: Sandboxed IPC sub-process to render previews.
- GUI: Add previews for pdf: Render the page the search keywords were found. Highlight the keywords when rendering the page.
- GUI: Add previews for plaintext files: Extract snippets. Highlight the keywords when rendering the page.
- General: Add basic filters for query.
- GUI: Add icon. Special thanks to the following sources:
	https://www.svgrepo.com/svg/151751/magnifier-with-small-handle
	https://www.svgrepo.com/svg/52764/open-book

	Thanks!
- General: Documentation.

- Add packages: Ubuntu 21.10, 22.04

Thanks to all those who provided feedback (and endured bugs) at various stages. You know who you are, thx!
