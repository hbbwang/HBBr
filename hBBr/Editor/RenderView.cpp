#include "RenderView.h"

#pragma comment(lib , "RendererCore.lib")

RenderView::RenderView(QWidget *parent)
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
	if (_vkRenderer != NULL)
		delete _vkRenderer;
	_vkRenderer = NULL;
}

void RenderView::showEvent(QShowEvent* event)
{
	if (_vkRenderer == NULL)
		_vkRenderer = new VulkanRenderer((void*)this->winId(), "MainRenderer" , true);
	_vkRenderer->ResetWindowSize(width(), height());
}

void RenderView::resizeEvent(QResizeEvent* event)
{
	if (_vkRenderer != NULL)
		_vkRenderer->ResetWindowSize(width(), height());
}

void RenderView::FuncRender()
{
	//ʹ����Ⱦ�߳�,��������ѭ��
	//if (_vkRenderer != NULL)
	//{
	//	_vkRenderer->Render();
	//}
}