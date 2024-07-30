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
}

void RenderView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
	//_sleep(1);
}

void RenderView::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
}

void RenderView::paintEvent(QPaintEvent* event)
{

}

void RenderView::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))//如果是来自Item的数据
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
				if (assets.capacity() <= assets.size())
				{
					assets.reserve(assets.capacity() + 25);
				}
				int row, col;
				QMap<int, QVariant> roleDataMap;
				stream >> row >> col >> roleDataMap;
				if (roleDataMap.contains(Qt::DisplayRole))
				{
					//收集拖拽的AssetInfos
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
