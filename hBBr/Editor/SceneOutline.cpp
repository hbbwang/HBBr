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
    if (_gameObject != NULL)
    {
        _gameObject->_editorObject = NULL;
        _gameObject->Destroy();
        _gameObject = NULL;
    }
    delete this;
}

SceneOutlineTree::SceneOutlineTree(class VulkanRenderer* renderer, QWidget* parent)
    :QTreeWidget(parent)
{
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setHeaderHidden(true);
    setIconSize({ 0,0 });
    //setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    //设置拖放模式为内部移动
    setDragDropMode(QAbstractItemView::InternalMove);
    //允许接受drop操作
    setAcceptDrops(true);
    //setEditTriggers(EditTrigger::DoubleClicked); 
    //setMouseTracking(true);
    setObjectName("SceneOutline");
    viewport()->setObjectName("SceneOutline");

    _renderer = renderer;
    _menu = new QMenu(this);
    _createNewGameObject    = new QAction(QString::fromLocal8Bit("创建GameObject"), _menu);
    _deleteGameObject       = new QAction(QString::fromLocal8Bit("删除"), _menu);
    _renameGameObject       = new QAction(QString::fromLocal8Bit("重命名"), _menu);

    _menu->addAction(_createNewGameObject);
    _menu->addAction(_renameGameObject);
    _menu->addSeparator();
    _menu->addAction(_deleteGameObject);

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
    connect(_renameGameObject, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([this]()
                {
                    if (this->currentItem())
                        editItem(this->currentItem());
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
    if (item->isExpanded())
    {
        item->setExpanded(false);
    }
    else
    {
        item->setExpanded(true);
    }
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
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setObjectName("SceneOutline");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(1,1,1,1);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);

    _search = new QLineEdit(this);
    mainLayout->addWidget(_search);

    _treeWidget = new SceneOutlineTree(renderer, this);
    mainLayout->addWidget(_treeWidget);

    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(0, 100);

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
        //QT5 QTreeWidgetItem 删除了父节点,子节点内存也会更着一起销毁,
        //会导致object->_editorObject(内存已被删除)获取出现异常,
        //所以销毁节点之前先把子节点取出，防止冲突。
        if (item->childCount() > 0)
        {
            if (item->parent())
            {
                item->parent()->addChildren(item->takeChildren());
            }
            else
            {
                _treeWidget->addTopLevelItems(item->takeChildren());
            }
        }
        item->Destroy();
    };
}

SceneOutline::~SceneOutline()
{

}

void SceneOutline::closeEvent(QCloseEvent* event)
{

}

void SceneOutline::focusInEvent(QFocusEvent* event)
{

}

