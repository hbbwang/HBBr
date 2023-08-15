#include "SceneOutline.h"
#include "VulkanRenderer.h"
#include "Resource/SceneManager.h"
#include <qheaderview.h>
#include <qaction.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QDrag>
GameObjectItem::GameObjectItem(GameObject* gameObject, QTreeWidget* view)
    :QTreeWidgetItem(view)
{
    _gameObject = gameObject;
    _gameObject->_editorObject = this;
    setText(0, gameObject->GetObjectName().c_str());
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

SceneOutlineTree::SceneOutlineTree(QWidget* parent)
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
    setMouseTracking(true);

    //setRootIsDecorated(false);
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(ItemDoubleClicked(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(sigEditFinished(QString)), this, SLOT(ItemEditFinished(QString)));
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
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    this->setLayout(mainLayout);

    _treeWidget = new SceneOutlineTree(this);
    mainLayout->addWidget(_treeWidget);

    renderer->GetScene()->_editorSceneUpdateFunc = [this]
    (SceneManager* scene, std::vector<std::shared_ptr<GameObject>> aliveObjects) 
    {

    };

    renderer->GetScene()->_editorGameObjectAddFunc = [this]
    (SceneManager* scene, std::shared_ptr<GameObject> object) 
    {
        _treeWidget->addTopLevelItem(new GameObjectItem(object.get(), _treeWidget));
    };

    renderer->GetScene()->_editorGameObjectRemoveFunc = [this]
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

