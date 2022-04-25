# looqs: Release notes


## 2022-0X-XX - v0.1
The first release comes with basic functionality. It's a start that can be considered useful to some degree.

Tested architectures: amd64.

CHANGES: 
- CLI command "looqs" to add/update/delete and search 
- GUI: "looqs-gui" to search, render previews, and add files
to index 
- General: Add multi-threaded indexing of all files (paths, mtime)
- General: sqlite based full-text search for: .txt,.pdf,.odt,.ods,.html...
- General: Sandboxed content processing
- GUI: Sandboxed GUI process, with privileged IPC daemon to allow launching external programs such as pdf viewers.
- GUI: Add previews for pdf: Render the page the search keywords were found. Highlight the keywords when rendering the page. 
- GUI: Add previews for plaintex tfiles: Extract snippets. Highlight the keywords when rendering the page. 
- General: Add basic filters for query.
- Add packages: Ubuntu 22.04

Thanks: All who provided feedback (and endured bugs). You know who you are, thx!
