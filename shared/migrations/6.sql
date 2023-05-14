CREATE TABLE outline(id INTEGER PRIMARY KEY, fileid INTEGER REFERENCES file (id) ON DELETE CASCADE, text varchar(1024), page integer);
CREATE INDEX outline_fileid ON outline (fileid);
