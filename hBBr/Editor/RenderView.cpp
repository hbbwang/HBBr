#include "RenderView.h"
#pragma comment(lib , "RendererCore.lib")
RenderView::RenderView(QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_PaintOnScreen);

	//_renderTimer = new QTimer(this);
	//_renderTimer->setInterval(1);
	//_renderTimer->start();
	//connect(_renderTimer,SIGNAL(timeout()),this,SLOT(FuncRender()));
}

RenderView::~RenderView()
{

}

void RenderView::showEvent(QShowEvent* event)
{
	if (_vkRenderer == NULL)
	{
		_vkRenderer = new VulkanRenderer((void*)this->winId(), "MainRenderer");
	}
	_vkRenderer->ResetWindowSize(width(), height());
}

void RenderView::resizeEvent(QResizeEvent* event)
{
	if (_vkRenderer != NULL)
		_vkRenderer->ResetWindowSize(width(), height());
}

void RenderView::closeEvent(QCloseEvent* event)
{
	_vkRenderer->Release();
	_vkRenderer = NULL;
}

void RenderView::FuncRender()
{
	//使用渲染线程,不在这里循环
	//if (_vkRenderer != NULL)
	//{
	//	_vkRenderer->Render();
	//}
}
