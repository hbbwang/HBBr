#include "RenderView.h"
#include "qstylepainter.h"
#include "QStyleOption.h"
#include "qevent.h"

#pragma comment(lib , "RendererCore.lib")
RenderView::RenderView(QWidget* parent)
	: QWidget(parent)
{
	//若是使用用户自定义绘制，则须要设置WA_PaintOnScreen
	setAttribute(Qt::WA_PaintOnScreen, true);
	//不须要默认的Qt背景
	setAttribute(Qt::WA_NoSystemBackground, true);
	//重绘时，绘制全部像素
	//setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoBackground, true);
	setAttribute(Qt::WA_NativeWindow, true);

	setStyleSheet("background-color:rgb(0,0,0);");
	//setMouseTracking(true);



	_renderTimer = new QTimer(this);
	_renderTimer->setInterval(1);
	connect(_renderTimer, SIGNAL(timeout()), this, SLOT(UpdateRender()));
	_renderTimer->start();
}

RenderView::~RenderView()
{

}

void RenderView::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}

void RenderView::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
}

bool RenderView::event(QEvent* event)
{
	return QWidget::event(event);
}


void RenderView::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);
}

void RenderView::paintEvent(QPaintEvent* event)
{
	QStylePainter painter(this);
	QStyleOption opt;
	opt.initFrom(this);
	opt.rect = rect();
	painter.drawPrimitive(QStyle::PE_Widget, opt);
}

void RenderView::UpdateRender()
{

}

void RenderView::focusInEvent(QFocusEvent* event)
{

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