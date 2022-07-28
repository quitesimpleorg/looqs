ALTER TABLE content ADD ftsid integer;
CREATE VIRTUAL TABLE fts USING fts5(content, content='');
DROP TRIGGER contents_ai;
DROP TRIGGER contents_au;
DROP TRIGGER contents_ad;
CREATE TEMP TABLE contentstemp(id INTEGER PRIMARY KEY, content text);
CREATE TRIGGER contentstemp_ai AFTER INSERT ON contentstemp BEGIN  INSERT INTO fts(content) VALUES (new.content); UPDATE content SET ftsid=last_insert_rowid() WHERE id = new.id; END;
INSERT INTO contentstemp(id, content) SELECT id, content FROM content;
DROP TRIGGER contentstemp_ai;
DROP TABLE contentstemp;
DROP TABLE content_fts;
ALTER TABLE content DROP COLUMN content;
CREATE INDEX content_ftsid ON content (ftsid);
CREATE TRIGGER content_ad AFTER DELETE ON content BEGIN  INSERT INTO fts(fts, rowid) VALUES('delete', old.ftsid); END;
