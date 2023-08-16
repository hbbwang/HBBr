#include "SceneOutline.h"
#include "VulkanRenderer.h"
#include "Resource/SceneManager.h"
#include "Component/GameObject.h"
#include <qheaderview.h>
#include <qaction.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QDrag>
#include <qmenu.h>
#include "EditorCommonFunction.h"
GameObjectItem::GameObjectItem(GameObject* gameObject, QTreeWidget* view)
    :QTreeWidgetItem(view)
{
    _gameObject = gameObject;
    _gameObject->_editorObject = this;
    setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
}

void GameObjectItem::Destroy()
{
    treeWidget()->indexOfTopLevelItem(this);
    _gameObject->_editorObject = NULL;
    this->_gameObject->Destroy();
    this->_gameObject = NULL;
    delete this;
}

SceneOutlineTree::SceneOutlineTree(class VulkanRenderer* renderer, QWidget* parent)
    :QTreeWidget(parent)
{
    setHeaderHidden(true);
    setIconSize({ 0,0 });
    //setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    //设置拖放模式为内部移动
    setDragDropMode(QAbstractItemView::InternalMove);
    //允许接受drop操作
    setAcceptDrops(true);
    setEditTriggers(EditTrigger::DoubleClicked); 
    //setMouseTracking(true);
    setObjectName("SceneOutline");

    _renderer = renderer;
    _menu = new QMenu(this);
    _createNewGameObject    = new QAction(QString::fromLocal8Bit("创建GameObject"), _menu);
    _deleteGameObject       = new QAction(QString::fromLocal8Bit("删除GameObject"), _menu);

    //setRootIsDecorated(false);
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(ItemDoubleClicked(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(sigEditFinished(QString)), this, SLOT(ItemEditFinished(QString)));
    connect(_createNewGameObject, &QAction::triggered, this, [this](bool bChecked) 
		{
            _renderer->ExecFunctionOnRenderThread([]() 
                {
                    GameObject::CreateGameObject();
                });           
		});
    connect(_deleteGameObject, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([this]()
                {
                    if (this->currentItem())
                    {
                        ((GameObjectItem*)this->currentItem())->_gameObject->Destroy();
                    }
                });
        });
}

void SceneOutlineTree::contextMenuEvent(QContextMenuEvent* event)
{
    _menu->addAction(_createNewGameObject);
    _menu->addSeparator();
    _menu->addAction(_deleteGameObject);
    _menu->exec(event->globalPos());
}

void SceneOutlineTree::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _curItem = this->itemAt(event->pos());
    }
    QTreeWidget::mousePressEvent(event);
}

void SceneOutlineTree::mouseMoveEvent(QMouseEvent* event)
{
    _mouseTouchItem = this->itemAt(event->pos());
    QTreeWidget::mouseMoveEvent(event);
}

void SceneOutlineTree::dropEvent(QDropEvent* event)
{
    QTreeWidget::dropEvent(event);
    if (_curItem != NULL)
    {
        GameObjectItem* dragItem = (GameObjectItem*)_curItem;
        if (_mouseTouchItem == NULL)
        {
            dragItem->_gameObject->SetParent(NULL);
        }
        else
        {
            GameObjectItem* currentItem = (GameObjectItem*)(_mouseTouchItem);
            {
                if (dragItem->parent())
                {
                    dragItem->_gameObject->SetParent(((GameObjectItem*)dragItem->parent())->_gameObject);
                }
                else
                {
                    dragItem->_gameObject->SetParent(NULL);
                }
            }
        }
        _curItem = NULL;
    }
}

void SceneOutlineTree::ItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    editItem(item);
}

void SceneOutlineTree::ItemEditFinished(QString newText)
{
    GameObjectItem* objItem = (GameObjectItem*)currentItem();
    if(objItem->_gameObject->GetObjectName() != newText.toStdString().c_str())
        objItem->_gameObject->SetObjectName(newText.toStdString().c_str());
}

SceneOutline::SceneOutline(VulkanRenderer* renderer, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("SceneOutline");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(1,1,1,1);
    this->setLayout(mainLayout);

    _treeWidget = new SceneOutlineTree(renderer, this);
    mainLayout->addWidget(_treeWidget);

    _renderer = renderer;
    auto scene = _renderer->GetScene();
    scene->_editorSceneUpdateFunc = [this, scene]
    (SceneManager* scene, std::vector<std::shared_ptr<GameObject>> aliveObjects)
    {

    };

    //Editor GameObject更新委托
    scene->_editorGameObjectUpdateFunc = [this, scene]
    (SceneManager* scene, std::shared_ptr<GameObject> object)
    {
        if (object->_bEditorNeedUpdate)
        {
            auto item = (GameObjectItem*)object->_editorObject;
            //rename?
            item->setText(0, object->GetObjectName().c_str());
        }
    };

    //Editor GameObject Spawn委托
    scene->_editorGameObjectAddFunc = [this, scene]
    (SceneManager* scene, std::shared_ptr<GameObject> object) 
    {
        _treeWidget->addTopLevelItem(new GameObjectItem(object.get(), _treeWidget));
    };

    //Editor GameObject Destroy委托
    scene->_editorGameObjectRemoveFunc = [this, scene]
    (SceneManager* scene, std::shared_ptr<GameObject> object)
    {
        auto item = (GameObjectItem*)object->_editorObject;
        item->Destroy();
    };
}

SceneOutline::~SceneOutline()
{

}

void SceneOutline::closeEvent(QCloseEvent* event)
{

}

