#include "RenderView.h"

#pragma comment(lib , "RendererCore.lib")

RenderView::RenderView(QWidget *parent)
	: QWidget(parent)
{
	if (_vkRenderer == NULL)
		_vkRenderer = new VulkanRenderer((void*)this->winId(),true);
	_renderTimer = new QTimer(this);
	_renderTimer->setInterval(1);
	_renderTimer->start();
	connect(_renderTimer,SIGNAL(timeout()),this,SLOT(FuncRender()));
}

RenderView::~RenderView()
{
	if (_vkRenderer != NULL)
		delete _vkRenderer;
	_vkRenderer = NULL;
}

void RenderView::FuncRender()
{
	if (_vkRenderer != NULL)
	{
		_vkRenderer->Render();
	}
}
