#include <QTextStream>
#include <QRegularExpression>

#include "previewgeneratorplaintext.h"
#include "previewresultplaintext.h"
#include "../shared/limitqueue.h"

QString PreviewGeneratorPlainText::generatePreviewText(QString content, RenderConfig config, QString fileName)
{
	QString resulText = "";

	QMap<int, QString> snippet;

	int coveredRange = -1;
	int lastWordPos = -1;

	QHash<QString, int> countmap;

	unsigned int currentSnippets = 0;
	for(QString &word : config.wordsToHighlight)
	{

		int lastPos = 0;
		int index = content.indexOf(word, lastPos, Qt::CaseInsensitive);
		while(index != -1 && currentSnippets < MAX_SNIPPETS)
		{
			countmap[word] = countmap.value(word, 0) + 1;

			if(index >= lastWordPos && index <= coveredRange)
			{
				break;
			}
			int begin = index - 50;
			if(begin < 0)
			{
				begin = 0;
			}
			int after = index + 50;
			if(after > content.size())
			{
				after = content.size();
			}

			snippet[index] = "...<br>" + content.mid(begin, after) + "...<br>";
			coveredRange = after;
			lastPos = index;

			index = content.indexOf(word, lastPos + 1, Qt::CaseInsensitive);
			++currentSnippets;
		}
		lastWordPos = lastPos;
	}

	auto i = snippet.constBegin();
	while(i != snippet.constEnd())
	{
		resulText.append(i.value());
		++i;
	}

	resulText = resulText.toHtmlEscaped();
	QString header = "<b>" + fileName + "</b> ";
	for(QString &word : config.wordsToHighlight)
	{
		resulText.replace(word, "<span style=\"background-color: yellow;\">" + word + "</span>", Qt::CaseInsensitive);
		header += word + ": " + QString::number(countmap[word]) + " ";
	}

	if(currentSnippets == MAX_SNIPPETS)
	{
		header += "(truncated)";
	}

	header += "<hr>";

	return header + resulText.replace("\n", "<br>").mid(0, 1000);
}

struct Snippet
{
	/* Contains each line number and line of the snippet*/
	QString snippetText;

	/* How many times a word occurs in the snippetText */
	QHash<QString, int> wordCountMap;
};

QString PreviewGeneratorPlainText::generateLineBasedPreviewText(QTextStream &in, RenderConfig config, QString fileName)
{
	QVector<Snippet> snippets;
	const int contextLinesCount = 2;
	LimitQueue<QString> queue(contextLinesCount);
	QString currentLine;
	currentLine.reserve(512);

	/* How many lines to read after a line with a match (like grep -A ) */
	int justReadLinesCount = -1;

	struct Snippet currentSnippet;

	auto appendLine = [&currentSnippet, &config](int lineNumber, QString &line)
	{
		int foundWordsCount = 0;
		for(QString &word : config.wordsToHighlight)
		{
			QRegularExpression searchRegex("\\b" + QRegularExpression::escape(word) + "\\b");
			bool containsRegex = line.contains(searchRegex);
			bool contains = false;
			if(!containsRegex)
			{
				contains = line.contains(word, Qt::CaseInsensitive);
			}
			if(containsRegex || contains)
			{
				currentSnippet.wordCountMap[word] = currentSnippet.wordCountMap.value(word, 0) + 1;
				QString replacementString = "<span style=\"background-color: yellow;\">" + word + "</span>";
				if(containsRegex)
				{
					line.replace(searchRegex, replacementString);
				}
				else
				{
					line.replace(word, replacementString, Qt::CaseInsensitive);
				}
				++foundWordsCount;
			}
		}
		currentSnippet.snippetText.append(QString("<b>%1</b>%2<br>").arg(lineNumber).arg(line));
		return foundWordsCount;
	};

	unsigned int lineCount = 0;
	while(in.readLineInto(&currentLine))
	{
		currentLine = currentLine.toHtmlEscaped();
		++lineCount;
		bool matched = false;
		if(justReadLinesCount > 0)
		{

			int result = appendLine(lineCount, currentLine);
			if(justReadLinesCount == 1 && result > 0)
			{
				justReadLinesCount = contextLinesCount;
			}
			else
			{
				--justReadLinesCount;
			}

			continue;
		}
		if(justReadLinesCount == 0)
		{
			currentSnippet.snippetText += "---<br>";
			justReadLinesCount = -1;
			snippets.append(currentSnippet);
			currentSnippet = {};
		}
		for(QString &word : config.wordsToHighlight)
		{
			if(currentLine.contains(word, Qt::CaseInsensitive))
			{
				matched = true;
				break;
			}
		}
		if(matched)
		{
			while(queue.size() > 0)
			{
				int queuedLineCount = lineCount - queue.size();
				QString queuedLine = queue.dequeue();
				appendLine(queuedLineCount, queuedLine);
			}
			appendLine(lineCount, currentLine);
			justReadLinesCount = contextLinesCount;
		}
		else
		{
			queue.enqueue(currentLine);
		}
	}

	if(!currentSnippet.snippetText.isEmpty())
	{
		currentSnippet.snippetText += "---<br>";
		snippets.append(currentSnippet);
	}

	std::sort(snippets.begin(), snippets.end(),
			  [](Snippet &a, Snippet &b)
			  {
				  int differentWordsA = 0;
				  int totalWordsA = 0;
				  int differentWordsB = 0;
				  int totalWordsB = 0;
				  for(int count : qAsConst(a.wordCountMap))
				  {
					  if(count > 0)
					  {
						  ++differentWordsA;
					  }
					  totalWordsA += count;
				  }
				  for(int count : qAsConst(b.wordCountMap))
				  {
					  if(count > 0)
					  {
						  ++differentWordsB;
					  }
					  totalWordsB += count;
				  }

				  if(differentWordsA > differentWordsB)
				  {
					  return true;
				  }
				  if(differentWordsA == differentWordsB)
				  {
					  return totalWordsA > totalWordsB;
				  }
				  return false;
			  });

	QString resultText = "";

	unsigned int snippetsCount = 0;

	QString header = "<b>" + fileName + "</b> ";

	QHash<QString, int> totalWordCountMap;
	bool isTruncated = false;
	for(Snippet &snippet : snippets)
	{
		if(snippetsCount++ < MAX_SNIPPETS)
		{
			resultText += snippet.snippetText;
		}
		else
		{
			isTruncated = true;
		}
		for(auto it = snippet.wordCountMap.keyValueBegin(); it != snippet.wordCountMap.keyValueEnd(); it++)
		{
			totalWordCountMap[it->first] = totalWordCountMap.value(it->first, 0) + it->second;
		}
	}
	if(isTruncated)
	{
		header += "(truncated) ";
	}
	for(QString &word : config.wordsToHighlight)
	{
		header += word + ": " + QString::number(totalWordCountMap[word]) + " ";
	}
	header += "<hr>";

	return header + resultText;
}

QSharedPointer<PreviewResult> PreviewGeneratorPlainText::generate(RenderConfig config, QString documentPath,
																  unsigned int page)
{
	PreviewResultPlainText *result = new PreviewResultPlainText(documentPath, page);
	QFile file(documentPath);
	if(!file.open(QFile::ReadOnly | QFile::Text))
	{
		return QSharedPointer<PreviewResultPlainText>(result);
	}
	QTextStream in(&file);
	QFileInfo info{documentPath};
	result->setText(generateLineBasedPreviewText(in, config, info.fileName()));
	return QSharedPointer<PreviewResultPlainText>(result);
}
