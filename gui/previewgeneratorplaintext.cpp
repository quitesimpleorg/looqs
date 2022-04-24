#include <QTextStream>

#include "previewgeneratorplaintext.h"
#include "previewresultplaintext.h"

PreviewResult *PreviewGeneratorPlainText::generate(RenderConfig config, QString documentPath, unsigned int page)
{
	PreviewResultPlainText *result = new PreviewResultPlainText(documentPath, page);
	QFile file(documentPath);
	if(!file.open(QFile::ReadOnly | QFile::Text))
	{
		return result;
	}
	QTextStream in(&file);

	QString resulText = "";
	QString content = in.readAll();
	QMap<int, QString> snippet;

	int coveredRange = 0;

	int lastWordPos = 0;
	QHash<QString, int> countmap;

	for(QString &word : config.wordsToHighlight)
	{

		int lastPos = 0;
		int index = content.indexOf(word, lastPos, Qt::CaseInsensitive);
		while(index != -1)
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

	QFileInfo info{documentPath};

	QString header = "<b>" + info.fileName() + "</b> ";
	for(QString &word : config.wordsToHighlight)
	{
		header += word + ": " + QString::number(countmap[word]) + " ";
	}
	header += "<hr>";

	result->setText(header + resulText.replace("\n", "<br>"));
	return result;
}
