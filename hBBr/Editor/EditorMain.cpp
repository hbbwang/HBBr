#include "EditorMain.h"
#include "RenderView.h"
#include "GLFWFormMain.h"
#include "GLFWInclude.h"
#include <qwindow.h>
EditorMain::EditorMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setFocusPolicy(Qt::ClickFocus);
}

EditorMain::~EditorMain()
{

}

void EditorMain::UpdateRender()
{
	auto windows = VulkanApp::GetWindows();
	for (int i = 0; i < windows.size(); i++)
	{
		if (glfwWindowShouldClose(windows[i].window))
		{
			VulkanApp::RemoveGLFWWindow(windows[i]);
			i = i - 1;
			continue;
		}
		else if (windows[i].renderer)
		{
			windows[i].renderer->Render();
		}
	}
}

VulkanGLFW* mainRenderer;

void EditorMain::showEvent(QShowEvent* event)
{
	if (_mainRenderer == NULL)
	{
		//Enable custom loop
		mainRenderer = VulkanApp::InitVulkanManager(false, true);

		//VulkanApp::SetSimpleGLFWWindow(*mainRenderer);
		HWND hwnd = (HWND)VulkanApp::GetHandle(*mainRenderer);
		auto mainRendererWindow = QWindow::fromWinId((WId)hwnd);
		_mainRenderer = QWidget::createWindowContainer(mainRendererWindow, this);
		setCentralWidget(_mainRenderer);

		_renderTimer = new QTimer(this);
		_renderTimer->setInterval(1);
		connect(_renderTimer, SIGNAL(timeout()), this, SLOT(UpdateRender()));
		_renderTimer->start();
	}
}

void EditorMain::closeEvent(QCloseEvent* event)
{
	_renderTimer->stop();
    VulkanApp::DeInitVulkanManager();
}

void EditorMain::resizeEvent(QResizeEvent* event)
{
}
