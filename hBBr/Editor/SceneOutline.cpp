#include "SceneOutline.h"
#include "VulkanRenderer.h"
#include "Resource/SceneManager.h"
#include <qheaderview.h>
#include <qaction.h>

GameObjectItem::GameObjectItem(GameObject* gameObject, QTreeWidget* view)
    :QTreeWidgetItem(view)
{
    _gameObject = gameObject;
    _gameObject->_editorObject = this;
    setText(0, gameObject->GetObjectName().c_str());
}

void GameObjectItem::Destroy()
{
    treeWidget()->indexOfTopLevelItem(this);
    _gameObject->_editorObject = NULL;
    this->_gameObject->Destroy();
    delete this;
}

SceneOutlineTree::SceneOutlineTree(QWidget* parent)
    :QTreeWidget(parent)
{
    setHeaderHidden(true);
    setIconSize({ 0,0 });
    setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    setEditTriggers(EditTrigger::DoubleClicked); 
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

