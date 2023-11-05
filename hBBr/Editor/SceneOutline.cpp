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
#include "CustomSearchLine.h"

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

    _menu_createBasic = new QMenu(this);
    _menu_createBasic->setTitle(QString::fromLocal8Bit("创建预设模型"));
    _createCube = new QAction(QString::fromLocal8Bit("Cube"), _menu_createBasic);
    _createSphere = new QAction(QString::fromLocal8Bit("Sphere"), _menu_createBasic);
    _createPlane = new QAction(QString::fromLocal8Bit("Plane"), _menu_createBasic);

    _menu_createBasic->addAction(_createCube);
    _menu_createBasic->addAction(_createSphere);
    _menu_createBasic->addAction(_createPlane);

    _menu->addAction(_createNewGameObject);
    _menu->addMenu(_menu_createBasic);
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
    //
    connect(_createCube, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Resource/Content/Core/Basic/Cube.FBX");
                    GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
                });
        });
    connect(_createSphere, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Resource/Content/Core/Basic/Sphere.FBX");
                    GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
                });
        });
    connect(_createPlane, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Resource/Content/Core/Basic/Plane.FBX");
                    GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
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
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);

    _search = new CustomSearchLine(this);
    _search->setMaximumHeight(30);
    _search->ui.comboBox->setHidden(true);
    mainLayout->addWidget(_search);
    connect(_search->ui.lineEdit, SIGNAL(returnPressed()), this, SLOT(TreeSearch()));

    _treeWidget = new SceneOutlineTree(renderer, this);
    mainLayout->addWidget(_treeWidget);

    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(0, 1000);

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

void traverse(QTreeWidgetItem* item , QList<QTreeWidgetItem*>& list , bool bHide = false , QString findName = "")
{
    // 遍历所有子节点
    for (int i = 0; i < item->childCount(); ++i) {
        traverse(item->child(i) , list , bHide);
        if (findName.length() > 0)
        {
            if (item->child(i)->text(0).contains(findName))
            {

            }
        }
        list.append(item->child(i));
        item->child(i)->setHidden(bHide);
    }
    item->setHidden(bHide);
}

bool searchTraverse(QTreeWidgetItem* item, QList<QTreeWidgetItem*>& list, QString findName = "")
{
    bool bHide = true;
    // 遍历所有子节点
    for (int i = 0; i < item->childCount(); ++i) {
        bHide = bHide && searchTraverse(item->child(i), list);
        if (findName.length() > 0)
        {
            if (item->child(i)->text(0).contains(findName))
            {
                bHide = false;
                continue;
            }
        }
        item->child(i)->setHidden(bHide);
        list.append(item->child(i));
    }

    if (findName.length() > 0)
    {
        if (item->text(0).contains(findName))
        {
            return false;
        }
    }

    return bHide;
}

void SceneOutline::TreeSearch()
{
    //_treeWidget->collapseAll();
    QList<QTreeWidgetItem*> searchList;
    _treeWidget->selectionModel()->clearSelection();
    if (_search->ui.lineEdit->text().isEmpty())
    {
        for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i) {
            traverse(_treeWidget->topLevelItem(i), searchList, false);
        }
        return;
    }
    QString input = _search->ui.lineEdit->text();
    _treeWidget->expandAll();
    //auto searchList = _treeWidget->findItems(input, Qt::MatchExactly)
    for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i) {
        bool bHide = searchTraverse(_treeWidget->topLevelItem(i) , searchList , input);
        _treeWidget->topLevelItem(i)->setHidden(bHide);       
    }
}

void SceneOutline::closeEvent(QCloseEvent* event)
{

}

void SceneOutline::focusInEvent(QFocusEvent* event)
{

}

