#include "RenderView.h"
#include "qstylepainter.h"
#include "QStyleOption.h"
#include "qevent.h"
#include <qwindow.h>
#include <qmessagebox.h>
#include "GLFWInclude.h"
#include "ConsoleDebug.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Asset/World.h"
#include "VulkanRenderer.h"
#ifdef _WIN32
#pragma comment(lib , "RendererCore.lib")
#endif

HWND hwnd;

RenderView::RenderView(QWidget* parent)
	: QWidget(parent)
{
	//若是使用用户自定义绘制，则须要设置WA_PaintOnScreen
	setAttribute(Qt::WA_PaintOnScreen, true);
	//不须要默认的Qt背景
	setAttribute(Qt::WA_NoSystemBackground, true);
	//重绘时，绘制全部像素
	setAttribute(Qt::WA_OpaquePaintEvent, true);

	setObjectName("RenderView");

	if (_mainRendererWidget == nullptr)
	{
		//Enable custom loop
		// 这个嵌入方法会导致窗口缩放闪黑,这是因为除了QT窗口，SDL自己也单独走了一次窗口缩放逻辑。
		_mainRenderer = VulkanApp::InitVulkanManager(false, true, (void*)this->winId());

		//下面这种嵌入方式可以解决上面的问题，但是会导致无法获取SDL的输入问题，这个还在解决，
		//实在不行就麻烦一点把QEvent解析成SDL的按键传过去...
		//_mainRenderer = VulkanApp::InitVulkanManager(false, true, nullptr);
		hwnd = (HWND)VulkanApp::GetWindowHandle(_mainRenderer);
		//{
		//	auto mainRendererWindow = QWindow::fromWinId((WId)hwnd);
		//	_mainRendererWidget = QWidget::createWindowContainer(mainRendererWindow, this);
		//	_mainRendererWidget->setFocusPolicy(Qt::StrongFocus);
		//	_mainRendererWidget->setObjectName("RenderView");
		//}


		auto dropFunc = [](VulkanForm *from, HString file) {
			//QMessageBox::information(0, from->name.c_str(), file.c_str(),0);
			ConsoleDebug::printf_endl(" [%s]Drop File : %s", from->name.c_str(), file.c_str());
			auto mainForm = VulkanApp::GetMainForm();
			if (from->renderer == mainForm->renderer && from->renderer->GetWorld())
			{
				auto assetInfo = ContentManager::Get()->GetAssetInfo(file);
				if (!assetInfo.expired())
				{
					GameObject* newObject = from->renderer->GetWorld()->SpawnGameObject(assetInfo.lock()->displayName);
					if (newObject)
					{
						auto modelComp = newObject->AddComponent<ModelComponent>();
						modelComp->SetModelByAssetPath(assetInfo.lock()->assetFilePath);
						newObject->SetObjectName(assetInfo.lock()->displayName);
					}
				}
			}
		};
		VulkanApp::AddDropCallback(dropFunc);
		
	}
}

RenderView::~RenderView()
{

}

void RenderView::Update()
{
	VulkanApp::UpdateForm();
}

void RenderView::showEvent(QShowEvent* event)
{

}

void RenderView::resizeEvent(QResizeEvent* event)
{
	if (_mainRenderer && _mainRenderer->renderer)
	{
		_mainRenderer->renderer->RendererResize(width(), height());
	}
	if (_mainRendererWidget != nullptr)
	{
		_mainRendererWidget->setGeometry(0, 0, width(), height());
	}
	QWidget::resizeEvent(event);

	//_sleep(1);
}

bool RenderView::event(QEvent* event)
{
	return QWidget::event(event);
}


void RenderView::closeEvent(QCloseEvent* event)
{
    if (hwnd)
    {
        VulkanApp::DeInitVulkanManager();
    }
}

void RenderView::paintEvent(QPaintEvent* event)
{
	//QStylePainter painter(this);
	//QStyleOption opt;
	//opt.initFrom(this);
	//opt.rect = rect();
	//painter.drawPrimitive(QStyle::PE_Widget, opt);
	//QWidget::paintEvent(event);
}

void RenderView::keyPressEvent(QKeyEvent* event)
{

}

void RenderView::keyReleaseEvent(QKeyEvent* event)
{

}

void RenderView::focusInEvent(QFocusEvent* event)
{
	//if (GetFocus() != hwnd || VulkanApp::GetFocusForm() != _mainRenderer)
	{
		//SetFocus(nullptr);
		VulkanApp::SetFocusForm(_mainRenderer);
		SetFocus(hwnd);
	}
}

void RenderView::focusOutEvent(QFocusEvent* event)
{

}

void RenderView::mousePressEvent(QMouseEvent* event)
{
	VulkanApp::SetFocusForm(_mainRenderer);
	SetFocus(hwnd);
}

void RenderView::mouseReleaseEvent(QMouseEvent* event)
{

}

SDL_Keycode RenderView::mapQtKeyToSdlKey(int qtKey)
{
    switch (qtKey) 
    {
        // 字母键
        case Qt::Key_A: return SDLK_a;
        case Qt::Key_B: return SDLK_b;
        case Qt::Key_C: return SDLK_c;
        case Qt::Key_D: return SDLK_d;
        case Qt::Key_E: return SDLK_e;
        case Qt::Key_F: return SDLK_f;
        case Qt::Key_G: return SDLK_g;
        case Qt::Key_H: return SDLK_h;
        case Qt::Key_I: return SDLK_i;
        case Qt::Key_J: return SDLK_j;
        case Qt::Key_K: return SDLK_k;
        case Qt::Key_L: return SDLK_l;
        case Qt::Key_M: return SDLK_m;
        case Qt::Key_N: return SDLK_n;
        case Qt::Key_O: return SDLK_o;
        case Qt::Key_P: return SDLK_p;
        case Qt::Key_Q: return SDLK_q;
        case Qt::Key_R: return SDLK_r;
        case Qt::Key_S: return SDLK_s;
        case Qt::Key_T: return SDLK_t;
        case Qt::Key_U: return SDLK_u;
        case Qt::Key_V: return SDLK_v;
        case Qt::Key_W: return SDLK_w;
        case Qt::Key_X: return SDLK_x;
        case Qt::Key_Y: return SDLK_y;
        case Qt::Key_Z: return SDLK_z;

            // 数字键
        case Qt::Key_0: return SDLK_0;
        case Qt::Key_1: return SDLK_1;
        case Qt::Key_2: return SDLK_2;
        case Qt::Key_3: return SDLK_3;
        case Qt::Key_4: return SDLK_4;
        case Qt::Key_5: return SDLK_5;
        case Qt::Key_6: return SDLK_6;
        case Qt::Key_7: return SDLK_7;
        case Qt::Key_8: return SDLK_8;
        case Qt::Key_9: return SDLK_9;

            // 功能键
        case Qt::Key_F1: return SDLK_F1;
        case Qt::Key_F2: return SDLK_F2;
        case Qt::Key_F3: return SDLK_F3;
        case Qt::Key_F4: return SDLK_F4;
        case Qt::Key_F5: return SDLK_F5;
        case Qt::Key_F6: return SDLK_F6;
        case Qt::Key_F7: return SDLK_F7;
        case Qt::Key_F8: return SDLK_F8;
        case Qt::Key_F9: return SDLK_F9;
        case Qt::Key_F10: return SDLK_F10;
        case Qt::Key_F11: return SDLK_F11;
        case Qt::Key_F12: return SDLK_F12;

            // 方向键
        case Qt::Key_Up: return SDLK_UP;
        case Qt::Key_Down: return SDLK_DOWN;
        case Qt::Key_Left: return SDLK_LEFT;
        case Qt::Key_Right: return SDLK_RIGHT;

            // 特殊键
        case Qt::Key_Escape: return SDLK_ESCAPE;
        case Qt::Key_Tab: return SDLK_TAB;
        case Qt::Key_Backspace: return SDLK_BACKSPACE;
        case Qt::Key_Return: return SDLK_RETURN;
        case Qt::Key_Enter: return SDLK_KP_ENTER;
        case Qt::Key_Insert: return SDLK_INSERT;
        case Qt::Key_Delete: return SDLK_DELETE;
        case Qt::Key_Pause: return SDLK_PAUSE;
        case Qt::Key_Print: return SDLK_PRINTSCREEN;
        case Qt::Key_SysReq: return SDLK_SYSREQ;
        case Qt::Key_Clear: return SDLK_CLEAR;
        case Qt::Key_Home: return SDLK_HOME;
        case Qt::Key_End: return SDLK_END;
        case Qt::Key_PageUp: return SDLK_PAGEUP;
        case Qt::Key_PageDown: return SDLK_PAGEDOWN;
        case Qt::Key_Space: return SDLK_SPACE;

            // 修饰键
        case Qt::Key_Shift: return SDLK_LSHIFT;
        case Qt::Key_Control: return SDLK_LCTRL;
            // 修饰键
        case Qt::Key_Alt: return SDLK_LALT;
        case Qt::Key_Meta: return SDLK_LGUI;
        case Qt::Key_AltGr: return SDLK_RALT;
        case Qt::Key_CapsLock: return SDLK_CAPSLOCK;
        case Qt::Key_NumLock: return SDLK_NUMLOCKCLEAR;
        case Qt::Key_ScrollLock: return SDLK_SCROLLLOCK;

            // 小键盘
        case Qt::Key_F13: return SDLK_KP_0;
        case Qt::Key_F14: return SDLK_KP_1;
        case Qt::Key_F15: return SDLK_KP_2;
        case Qt::Key_F16: return SDLK_KP_3;
        case Qt::Key_F17: return SDLK_KP_4;
        case Qt::Key_F18: return SDLK_KP_5;
        case Qt::Key_F19: return SDLK_KP_6;
        case Qt::Key_F20: return SDLK_KP_7;
        case Qt::Key_F21: return SDLK_KP_8;
        case Qt::Key_F22: return SDLK_KP_9;
        case Qt::Key_F23: return SDLK_KP_PERIOD;
        case Qt::Key_F24: return SDLK_KP_PLUS;
        case Qt::Key_F25: return SDLK_KP_MINUS;
        case Qt::Key_F26: return SDLK_KP_MULTIPLY;
        case Qt::Key_F27: return SDLK_KP_DIVIDE;
        case Qt::Key_F28: return SDLK_KP_ENTER;

            // 其他特殊键
        case Qt::Key_Menu: return SDLK_MENU;
        case Qt::Key_Help: return SDLK_HELP;
        case Qt::Key_Back: return SDLK_AC_BACK;
        case Qt::Key_Forward: return SDLK_AC_FORWARD;
        case Qt::Key_Stop: return SDLK_AC_STOP;
        case Qt::Key_Refresh: return SDLK_AC_REFRESH;
        case Qt::Key_VolumeDown: return SDLK_AUDIOMUTE;
        case Qt::Key_VolumeMute: return SDLK_VOLUMEDOWN;
        case Qt::Key_VolumeUp: return SDLK_VOLUMEUP;

            // 添加其他需要的按键映射
            // ...

        default: return SDLK_UNKNOWN;
    }
}