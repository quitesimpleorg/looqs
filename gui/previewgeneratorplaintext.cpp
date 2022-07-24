#include <QTextStream>

#include "previewgeneratorplaintext.h"
#include "previewresultplaintext.h"

QString PreviewGeneratorPlainText::generatePreviewText(QString content, RenderConfig config, QString fileName)
{
	QString resulText = "";

	QMap<int, QString> snippet;

	int coveredRange = -1;
	int lastWordPos = -1;

	QHash<QString, int> countmap;

	const unsigned int maxSnippets = 7;
	unsigned int currentSnippets = 0;
	for(QString &word : config.wordsToHighlight)
	{

		int lastPos = 0;
		int index = content.indexOf(word, lastPos, Qt::CaseInsensitive);
		while(index != -1 && currentSnippets < maxSnippets)
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

	for(QString &word : config.wordsToHighlight)
	{
		resulText.replace(word, "<span style=\"background-color: yellow;\">" + word + "</span>", Qt::CaseInsensitive);
	}

	QString header = "<b>" + fileName + "</b> ";
	for(QString &word : config.wordsToHighlight)
	{
		header += word + ": " + QString::number(countmap[word]) + " ";
	}
	if(currentSnippets == maxSnippets)
	{
		header += "(truncated)";
	}

	header += "<hr>";

	return header + resulText.replace("\n", "<br>").mid(0, 1000);
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

	QString content = in.readAll();
	QFileInfo info{documentPath};
	result->setText(generatePreviewText(content, config, info.fileName()));
	return QSharedPointer<PreviewResultPlainText>(result);
}
