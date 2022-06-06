# looqs: Release notes

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
