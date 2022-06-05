# looqs: Release notes

## 2022-06-XX - v0.1
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
- Add packages: Ubuntu 22.04

Thanks to all those who provided feedback (and endured bugs) at various stages. You know who you are, thx!
