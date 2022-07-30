# looqs: Release notes
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
