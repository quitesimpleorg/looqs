CREATE VIRTUAL TABLE fts_trigram USING fts5(content, content='',tokenize="trigram");
ALTER TABLE content ADD COLUMN fts_trigramid integer;
CREATE INDEX content_fts_trigramid ON content (fts_trigramid);
