#include "SDLWidget.h"

VulkanForm* SDLWidget::_mainRenderer = nullptr;
SDLWidget::SDLWidget(QWidget* parent)
	:QWidget(parent)
{
	//若是使用用户自定义绘制，则须要设置WA_PaintOnScreen
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	//务必关闭QT5的更新，不然在刷新窗口的时候会有一瞬间顶在SDL的上面，造成闪烁。
	setUpdatesEnabled(false);
	setObjectName("SDLRenderer_Main");
	_rendererForm = VulkanApp::InitVulkanManager(false, true, (void*)this->winId());
	_hwnd = (HWND)VulkanApp::GetWindowHandle(_rendererForm);
}

SDLWidget::SDLWidget(QWidget* parent, QString titleName)
	:QWidget(parent)
{
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setUpdatesEnabled(false);
	setObjectName("SDLRenderer");
	_rendererForm = VulkanApp::CreateNewWindow(512, 512, titleName.toStdString().c_str(), true, (void*)this->winId());
	_hwnd = (HWND)VulkanApp::GetWindowHandle(_rendererForm);
}

SDLWidget::~SDLWidget()
{
}

void SDLWidget::RendererUpdate()
{
}
