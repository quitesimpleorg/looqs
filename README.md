easyindex
=========
easyindex creates a poor-man full-text search for your files using a
sqlite database.

You need the python "chardet" package, since it will try to convert the 
encoding of the files in case initial utf-8 decoding fails.

pdftext is needed to search in .pdf files..

No GUI is provided at this time, nor does it concern itself with search
too much. 

Setup
-----
sqlite3 easyindex.db < create.sql


