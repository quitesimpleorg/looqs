-- Create a table. And an external content fts5 table to index it.
CREATE TABLE file(id INTEGER PRIMARY KEY, path varchar(4096) UNIQUE, mtime integer, content text);
CREATE VIRTUAL TABLE file_fts USING fts5(content, content='file', content_rowid='id');

-- Triggers to keep the FTS index up to date.
CREATE TRIGGER file_ai AFTER INSERT ON file BEGIN
  INSERT INTO file_fts(rowid, content) VALUES (new.id, new.content);
END;
CREATE TRIGGER file_ad AFTER DELETE ON file BEGIN
  INSERT INTO file_fts(file_fts, rowid, content) VALUES('delete', old.id, old.content);
END;
CREATE TRIGGER file_au AFTER UPDATE ON file BEGIN
  INSERT INTO file_fts(file_fts, rowid, content) VALUES('delete', old.id, old.content);
  INSERT INTO file_fts(rowid, content) VALUES (new.id, new.content);
END;
