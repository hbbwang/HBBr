#include "RenderView.h"
#include "qstylepainter.h"
#include "QStyleOption.h"
#include "qevent.h"
#include <qwindow.h>

#include "FormMain.h"
#include "GLFWInclude.h"

#pragma comment(lib , "RendererCore.lib")

RenderView::RenderView(QWidget* parent)
	: QWidget(parent)
{
	//若是使用用户自定义绘制，则须要设置WA_PaintOnScreen
	setAttribute(Qt::WA_PaintOnScreen, true);
	//不须要默认的Qt背景
	setAttribute(Qt::WA_NoSystemBackground, true);
	//重绘时，绘制全部像素
	setAttribute(Qt::WA_OpaquePaintEvent, true);

	//setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);
	setObjectName("RenderView");

	_renderTimer = new QTimer(this);
	_renderTimer->setInterval(1);
	connect(_renderTimer, SIGNAL(timeout()), this, SLOT(UpdateRender()));
	_renderTimer->start();


	if (_mainRendererWidget == NULL)
	{
		//Enable custom loop
		_mainRenderer = VulkanApp::InitVulkanManager(false, true);

		HWND hwnd = (HWND)VulkanApp::GetWindowHandle(_mainRenderer);
		auto mainRendererWindow = QWindow::fromWinId((WId)hwnd);
		_mainRendererWidget = QWidget::createWindowContainer(mainRendererWindow, this);
		_mainRendererWidget->setFocusPolicy(Qt::ClickFocus);
		_mainRendererWidget->setObjectName("RenderView");

		_renderTimer = new QTimer(this);
		_renderTimer->setInterval(1);
		connect(_renderTimer, SIGNAL(timeout()), this, SLOT(UpdateRender()));
		_renderTimer->start();
	}

}

RenderView::~RenderView()
{

}

void RenderView::showEvent(QShowEvent* event)
{

}

void RenderView::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	if (_mainRendererWidget != NULL)
	{
		_mainRendererWidget->setGeometry(0, 0, width(), height());
	}
	_sleep(1);
}

bool RenderView::event(QEvent* event)
{
	return QWidget::event(event);
}


void RenderView::closeEvent(QCloseEvent* event)
{
	_renderTimer->stop();
	VulkanApp::DeInitVulkanManager();
}

void RenderView::paintEvent(QPaintEvent* event)
{
	QStylePainter painter(this);
	QStyleOption opt;
	opt.initFrom(this);
	opt.rect = rect();
	painter.drawPrimitive(QStyle::PE_Widget, opt);

	QWidget::paintEvent(event);
}

void RenderView::UpdateRender()
{
	VulkanApp::UpdateForm();
}

void RenderView::focusInEvent(QFocusEvent* event)
{
	VulkanApp::SetFormFocus(_mainRenderer);
}

void RenderView::focusOutEvent(QFocusEvent* event)
{
}

void RenderView::mousePressEvent(QMouseEvent* event)
{

}

void RenderView::mouseReleaseEvent(QMouseEvent* event)
{

}