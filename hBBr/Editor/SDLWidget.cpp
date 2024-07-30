#include "SDLWidget.h"
#include "ShaderCompiler.h"
#include "EditorCommonFunction.h"
#include <functional>
#include "FormMain.h"
#include <qwindow.h>

VulkanForm* SDLWidget::_mainRenderer = nullptr;
SDLWidget::SDLWidget(QWidget* parent)
	:QWidget(parent)
{
	//��عر�QT5�ĸ��£���Ȼ��ˢ�´��ڵ�ʱ�����һ˲�䶥��SDL�����棬�����˸��
	setUpdatesEnabled(false);
	//����ʹ���û��Զ�����ƣ�����Ҫ����WA_PaintOnScreen
	setAttribute(Qt::WA_ForceUpdatesDisabled, true);
	setAttribute(Qt::WA_StaticContents, true);
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_UpdatesDisabled, true);
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
	setUpdatesEnabled(false);
	setAttribute(Qt::WA_ForceUpdatesDisabled, true);
	setAttribute(Qt::WA_StaticContents, true);
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_UpdatesDisabled, true);
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
	if (VulkanApp::GetForms().size() <= 0)
	{
		VulkanApp::DeInitVulkanManager();
	}
}

void SDLWidget::RendererUpdate()
{
}

void SDLWidget::resizeEvent(QResizeEvent* event)
{
}
