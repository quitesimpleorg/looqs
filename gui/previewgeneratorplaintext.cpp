#include <QTextStream>

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

QString PreviewGeneratorPlainText::generateLineBasedPreviewText(QTextStream &in, RenderConfig config, QString fileName)
{
	QString resultText;
	const unsigned int contextLinesCount = 2;
	LimitQueue<QString> queue(contextLinesCount);
	QString currentLine;
	currentLine.reserve(512);

	/* How many lines to read after a line with a match (like grep -A ) */
	int justReadLinesCount = -1;

	auto appendLine = [&resultText](int lineNumber, QString &line)
	{ resultText.append(QString("<b>%1</b>%2<br>").arg(lineNumber).arg(line)); };

	QHash<QString, int> countmap;
	QString header = "<b>" + fileName + "</b> ";

	unsigned int snippetsCount = 0;
	unsigned int lineCount = 0;
	while(in.readLineInto(&currentLine) && snippetsCount < MAX_SNIPPETS)
	{
		++lineCount;
		bool matched = false;
		if(justReadLinesCount > 0)
		{
			appendLine(lineCount, currentLine);
			--justReadLinesCount;
			continue;
		}
		if(justReadLinesCount == 0)
		{
			resultText += "---<br>";
			justReadLinesCount = -1;
			++snippetsCount;
		}
		for(QString &word : config.wordsToHighlight)
		{
			if(currentLine.contains(word, Qt::CaseInsensitive))
			{
				countmap[word] = countmap.value(word, 0) + 1;
				matched = true;
				currentLine.replace(word, "<span style=\"background-color: yellow;\">" + word + "</span>",
									Qt::CaseInsensitive);
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

	for(QString &word : config.wordsToHighlight)
	{
		header += word + ": " + QString::number(countmap[word]) + " ";
	}
	if(snippetsCount == MAX_SNIPPETS)
	{
		header += "(truncated)";
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
