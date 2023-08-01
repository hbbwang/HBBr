#include"GameObject.h"
#include "FormMain.h"
#include "Resource/SceneManager.h"

GameObject::GameObject(SceneManager* scene)
{
	if (scene == NULL)
	{
		_scene = VulkanApp::GetMainForm()->renderer->GetScene();
	}
	else
	{
		_scene = scene;
	}
	_scene->_gameObjects.push_back(this);
}

GameObject::~GameObject()
{

}

void GameObject::SetActive(bool newActive)
{
	_bActive = newActive;
	if (!newActive)//When object disable,what's going to happen?
	{

	}
}

void GameObject::Init()
{

}

bool GameObject::Update()
{
	if (_bWantDestroy)
	{
		auto it = std::remove(_scene->_gameObjects.begin(), _scene->_gameObjects.end(), this);
		if (it != _scene->_gameObjects.end())
		{
			_scene->_gameObjects.erase(it);
		}
		return false;
	}
	else
	{
		if (_bActive)
		{

		}
	}
}
