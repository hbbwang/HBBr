#include "RenderView.h"
#include "qstylepainter.h"
#include "QStyleOption.h"
#include "qevent.h"
#include <qwindow.h>
#include <qmessagebox.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <qmimedata.h>
#include "GLFWInclude.h"
#include "ConsoleDebug.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "ContentBrowser.h"
#include "EditorMain.h"
#include "SceneOutline.h"
#include "Asset/World.h"
#include "VulkanRenderer.h"
#include "EditorCommonFunction.h"
#include <qdebug.h>
#include "ShaderCompiler.h"
#ifdef _WIN32
#pragma comment(lib , "RendererCore.lib")
#endif

RenderView::RenderView(QWidget* parent)
	: QWidget(parent)
{
	setObjectName("RenderView");
    setAcceptDrops(true);

    setLayout(new QHBoxLayout(this));
    ((QHBoxLayout*)layout())->setContentsMargins(0,0,0,0);
    ((QHBoxLayout*)layout())->setSpacing(0);

    _mainRendererWidget = new SDLWidget(this);
    layout()->addWidget(_mainRendererWidget);
}

RenderView::~RenderView()
{
}

void RenderView::Update()
{
    _mainRendererWidget->RendererUpdate();
	VulkanApp::UpdateForm();
}

void RenderView::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	//_sleep(1);
}

void RenderView::closeEvent(QCloseEvent* event)
{
    VulkanApp::DeInitVulkanManager();
}

void RenderView::paintEvent(QPaintEvent* event)
{
}

void RenderView::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))//���������Item������
    {
        e->acceptProposedAction();
    }
}

void RenderView::dropEvent(QDropEvent* e)
{
	//Get Items
	{
		QByteArray encoded = e->mimeData()->data("application/x-qabstractitemmodeldatalist");
        auto cb = ContentBrowser::GetContentBrowsers();
        auto source = dynamic_cast<VirtualFileListView*>(e->source());
		if (!encoded.isEmpty() && source && cb.size()>0)
		{
			std::vector<AssetInfoBase*> assets;
			QDataStream stream(&encoded, QIODevice::ReadOnly);
			while (!stream.atEnd())
			{
				if (assets.capacity() < assets.size())
				{
					assets.reserve(assets.capacity() + 25);
				}
				int row, col;
				QMap<int, QVariant> roleDataMap;
				stream >> row >> col >> roleDataMap;
				if (roleDataMap.contains(Qt::DisplayRole))
				{
					//�ռ���ק��AssetInfos
					if (e->source()->objectName().compare(cb[0]->_treeView->objectName()) == 0)//Tree View
					{
						//QMessageBox::information(0, 0, "Tree View", 0);
					}
					else if (e->source()->objectName().compare(source->objectName()) == 0)//List View
					{
                        //QMessageBox::information(0, 0, "List View", 0);
                        CustomListItem* Item =(CustomListItem*)source->item(row);
                        if (Item && !Item->_assetInfo.expired())
                        {
                            const auto info = Item->_assetInfo.lock().get();
                            auto world = _mainRendererWidget->_rendererForm->renderer->GetWorld();
                            HString name = Item->_assetInfo.lock()->displayName;
                            if (info->type == AssetType::Model)
                            {
                                Level* level = nullptr;
                                if (EditorMain::_self->_sceneOutline->_currentLevelItem && !EditorMain::_self->_sceneOutline->_currentLevelItem->_level.expired())
                                {
                                    level = EditorMain::_self->_sceneOutline->_currentLevelItem->_level.lock().get();
                                }
                                if (level)
                                {
                                    auto gameObject = _mainRendererWidget->_rendererForm->renderer->GetWorld().lock()->SpawnGameObject(name, level);
                                    level->MarkDirty();
                                    if (gameObject)
                                    {
                                        auto modelComp = gameObject->AddComponent<ModelComponent>();
                                        modelComp->SetModelByAssetPath(Item->_assetInfo.lock()->virtualFilePath);
                                    }
                                }
                                else
                                {
                                    ConsoleDebug::print_endl(GetEditorInternationalization("RenderView","SpawnGameObject").toStdString().c_str(),"255,255,0");
                                }
                             
                            }                           
                        }
					}
				}
			}
		}
	}
}

SDL_Keycode RenderView::mapQtKeyToSdlKey(int qtKey)
{
    switch (qtKey) 
    {
        // ��ĸ��
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

            // ���ּ�
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

            // ���ܼ�
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

            // �����
        case Qt::Key_Up: return SDLK_UP;
        case Qt::Key_Down: return SDLK_DOWN;
        case Qt::Key_Left: return SDLK_LEFT;
        case Qt::Key_Right: return SDLK_RIGHT;

            // �����
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

            // ���μ�
        case Qt::Key_Shift: return SDLK_LSHIFT;
        case Qt::Key_Control: return SDLK_LCTRL;
            // ���μ�
        case Qt::Key_Alt: return SDLK_LALT;
        case Qt::Key_Meta: return SDLK_LGUI;
        case Qt::Key_AltGr: return SDLK_RALT;
        case Qt::Key_CapsLock: return SDLK_CAPSLOCK;
        case Qt::Key_NumLock: return SDLK_NUMLOCKCLEAR;
        case Qt::Key_ScrollLock: return SDLK_SCROLLLOCK;

            // С����
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

            // ���������
        case Qt::Key_Menu: return SDLK_MENU;
        case Qt::Key_Help: return SDLK_HELP;
        case Qt::Key_Back: return SDLK_AC_BACK;
        case Qt::Key_Forward: return SDLK_AC_FORWARD;
        case Qt::Key_Stop: return SDLK_AC_STOP;
        case Qt::Key_Refresh: return SDLK_AC_REFRESH;
        case Qt::Key_VolumeDown: return SDLK_AUDIOMUTE;
        case Qt::Key_VolumeMute: return SDLK_VOLUMEDOWN;
        case Qt::Key_VolumeUp: return SDLK_VOLUMEUP;

            // ���������Ҫ�İ���ӳ��
            // ...

        default: return SDLK_UNKNOWN;
    }
}