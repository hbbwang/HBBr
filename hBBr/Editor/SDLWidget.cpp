#include "SDLWidget.h"
#include "ShaderCompiler.h"
#include "EditorCommonFunction.h"

VulkanForm* SDLWidget::_mainRenderer = nullptr;
SDLWidget::SDLWidget(QWidget* parent)
	:QWidget(parent)
{
	//����ʹ���û��Զ�����ƣ�����Ҫ����WA_PaintOnScreen
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	//��عر�QT5�ĸ��£���Ȼ��ˢ�´��ڵ�ʱ�����һ˲�䶥��SDL�����棬�����˸��
	setUpdatesEnabled(false);
	setObjectName("SDLRenderer_Main");

	//Global setting
	VulkanApp::SetEditorVulkanInit(
		[]() {
			Shaderc::ShaderCompiler::SetEnableShaderDebug(GetEditorConfigInt("Default", "EnableShaderDebug"));
	
		
		});
	//
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
