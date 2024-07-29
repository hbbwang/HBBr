#include "SDLWidget.h"
#include "ShaderCompiler.h"
#include "EditorCommonFunction.h"
#include <functional>
#include "FormMain.h"

VulkanForm* SDLWidget::_mainRenderer = nullptr;
SDLWidget::SDLWidget(QWidget* parent)
	:QWidget(parent)
{
	//若是使用用户自定义绘制，则须要设置WA_PaintOnScreen
	setAttribute(Qt::WA_ForceUpdatesDisabled, true);
	setAttribute(Qt::WA_StaticContents, true);
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_UpdatesDisabled, true);
	//务必关闭QT5的更新，不然在刷新窗口的时候会有一瞬间顶在SDL的上面，造成闪烁。
	setUpdatesEnabled(false);
	setObjectName("SDLRenderer_Main");

	//Global setting
	VulkanApp::SetEditorVulkanInit(
		[]() {
			Shaderc::ShaderCompiler::SetEnableShaderDebug(GetEditorConfigInt("Default", "EnableShaderDebug"));
			
		});
	//
	_rendererForm = VulkanApp::InitVulkanManager(false, true, (void*)this->winId());
	_hwnd = (HWND)VulkanApp::GetWindowHandle(_rendererForm->window);

	VulkanApp::SetFocusForm(_rendererForm);
	SetFocus(_hwnd);
}

SDLWidget::SDLWidget(QWidget* parent, QString titleName)
	:QWidget(parent)
{
	setAttribute(Qt::WA_ForceUpdatesDisabled, true);
	setAttribute(Qt::WA_StaticContents, true);
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_UpdatesDisabled, true);
	setUpdatesEnabled(false);
	setObjectName("SDLRenderer");
	_rendererForm = VulkanApp::CreateNewWindow(512, 512, titleName.toStdString().c_str(), true, (void*)this->winId());
	_hwnd = (HWND)VulkanApp::GetWindowHandle(_rendererForm->window);

	VulkanApp::SetFocusForm(_rendererForm);
	SetFocus(_hwnd);
}

SDLWidget::~SDLWidget()
{
	for (auto& i : VulkanApp::GetForms())
	{
		if (i == _rendererForm && _rendererForm && _rendererForm->renderer)
		{
			VulkanApp::RemoveWindow(_rendererForm);
			_rendererForm->renderer = nullptr;
		}
	}
}

void SDLWidget::RendererUpdate()
{
}
