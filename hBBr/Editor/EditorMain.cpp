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
    ////Main render view
    //_mainRenderView = new RenderView(this);
    ////SetParent((HWND)_mainRenderView->winId() , (HWND)winId());
    //setCentralWidget(_mainRenderView);
    //centralWidget()->setFocus();
    //centralWidget()->setFocusPolicy(Qt::ClickFocus);

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
			auto window = windows[i].window;
			auto it = std::remove_if(windows.begin(), windows.end(), [window](VulkanGLFW& glfw) {
				return window == glfw.window;
				});
			if (it != windows.end())
			{
				windows.erase(it);
			}
			i = i - 1;
			continue;
		}
		else if (windows[i].renderer)
		{
			windows[i].renderer->Render();
		}
	}
}

void EditorMain::showEvent(QShowEvent* event)
{
	if (_mainRenderer == NULL)
	{
		//Enable custom loop
		auto mainRenderer = VulkanApp::InitVulkanManager(false, true);
		HWND hwnd = glfwGetWin32Window(mainRenderer);
		auto mainRendererWindow = QWindow::fromWinId((WId)hwnd);
		_mainRenderer = QWidget::createWindowContainer(mainRendererWindow, this);
		//SetParent(hwnd, (HWND)winId());
		_renderTimer = new QTimer(this);
		_renderTimer->setInterval(1);
		connect(_renderTimer, SIGNAL(timeout()), this, SLOT(UpdateRender()));
		_renderTimer->start();
	}
}

void EditorMain::closeEvent(QCloseEvent* event)
{
	_mainRenderer->close();
    VulkanApp::DeInitVulkanManager();
}

void EditorMain::focusInEvent(QFocusEvent* event)
{
}

void EditorMain::focusOutEvent(QFocusEvent* event)
{
}

void EditorMain::mousePressEvent(QMouseEvent* event)
{
}

void EditorMain::mouseReleaseEvent(QMouseEvent* event)
{
}

void EditorMain::resizeEvent(QResizeEvent* event)
{
}

bool EditorMain::eventFilter(QObject* watched, QEvent* event)
{
    return QMainWindow::eventFilter(watched, event);
}
