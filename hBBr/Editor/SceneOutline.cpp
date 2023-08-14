#include "SceneOutline.h"
#include "VulkanRenderer.h"
#include "Resource/SceneManager.h"

void SceneUpdate(SceneManager* scene, std::vector<std::shared_ptr<GameObject>> aliveObjects)
{

}

void GameObjectAdd(SceneManager* scene, std::shared_ptr<GameObject> object)
{

}

void GameObjectRemove(SceneManager* scene, std::shared_ptr<GameObject> object)
{

}

SceneOutline::SceneOutline(VulkanRenderer* renderer, QWidget *parent)
    : QWidget(parent)
{
    renderer->GetScene()->_editorUpdateFunc = SceneUpdate;
    renderer->GetScene()->_editorGameObjectAddFunc = GameObjectAdd;
    renderer->GetScene()->_editorGameObjectRemoveFunc = GameObjectRemove;
    //

}

SceneOutline::~SceneOutline()
{

}

void SceneOutline::closeEvent(QCloseEvent* event)
{

}