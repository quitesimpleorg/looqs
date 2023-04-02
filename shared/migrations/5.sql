CREATE TABLE tag(id integer PRIMARY KEY, name varchar(128) UNIQUE);
CREATE TABLE filetag(fileid integer, tagid integer);
CREATE INDEX filetag_fileid ON filetag(fileid);
CREATE INDEX tag_id ON tag(id);
CREATE INDEX file_path ON file ( path );
UPDATE file SET filetype='c' WHERE filetype='f';
