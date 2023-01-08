#include "previewcoordinator.h"
#include <QFileInfo>

PreviewCoordinator::PreviewCoordinator()
{
	this->ipcPreviewClient.moveToThread(&this->ipcClientThread);

	connect(&ipcPreviewClient, &IPCPreviewClient::previewReceived, this, &PreviewCoordinator::handleReceivedPreview,
			Qt::QueuedConnection);
	connect(&ipcPreviewClient, &IPCPreviewClient::finished, this, [&] { emit completedGeneration(); });
	connect(this, &PreviewCoordinator::ipcStartGeneration, &ipcPreviewClient, &IPCPreviewClient::startGeneration,
			Qt::QueuedConnection);

	this->ipcClientThread.start();
}

void PreviewCoordinator::init(const QVector<SearchResult> &searchResults)
{
	this->previewableSearchResults.clear();
	for(const SearchResult &result : searchResults)
	{
		if(result.wasContentSearch)
		{
			QString path = result.fileData.absPath;
			// HACK until we can preview them properly
			if(path.endsWith(".html") || path.endsWith(".htm"))
			{
				continue;
			}
			QFileInfo info{path};
			if(info.exists())
			{
				this->previewableSearchResults.append(result);
			}
		}
	}
}

void PreviewCoordinator::setSocketPath(QString socketPath)
{
	this->socketPath = socketPath;
	this->ipcPreviewClient.setSocketPath(socketPath);
}

int PreviewCoordinator::previewableCount() const
{
	return this->previewableSearchResults.count();
}

QSharedPointer<PreviewResult> PreviewCoordinator::resultAt(int index)
{
	if(this->previewResults.size() > index)
	{
		return {this->previewResults[index]};
	}
	return {nullptr};
}

const QVector<SearchResult> &PreviewCoordinator::getPreviewableSearchResults() const
{
	return this->previewableSearchResults;
}

void PreviewCoordinator::handleReceivedPreview(QSharedPointer<PreviewResult> preview, unsigned int previewGeneration)
{
	if(previewGeneration < this->currentPreviewGeneration)
	{
		return;
	}
	if(!preview.isNull() && preview->hasPreview())
	{
		QString docPath = preview->getDocumentPath();
		auto previewPage = preview->getPage();
		int pos = previewOrder[docPath + QString::number(previewPage)];
		this->previewResults[pos] = preview;
		emit previewReady();
	}
}

void PreviewCoordinator::startGeneration(RenderConfig config, const QVector<RenderTarget> &targets)
{
	++this->currentPreviewGeneration;

	this->previewOrder.clear();
	this->previewResults.clear();

	this->previewResults.resize(targets.size());
	this->previewResults.fill(nullptr);

	int i = 0;
	for(const RenderTarget &target : targets)
	{
		this->previewOrder[target.path + QString::number(target.page)] = i++;
	}

	emit ipcStartGeneration(config, targets);
}
