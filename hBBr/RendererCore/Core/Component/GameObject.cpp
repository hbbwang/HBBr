#include"GameObject.h"
#include "FormMain.h"
#include "Resource/SceneManager.h"
#include "Component.h"
GameObject::GameObject(HString objectName, SceneManager* scene)
{
	if (scene == NULL)
	{
		_scene = VulkanApp::GetMainForm()->renderer->GetScene();
	}
	else
	{
		_scene = scene;
	}
	_name = objectName;
	_bActive = true;
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
	_bInit = true;
}

bool GameObject::Update()
{
	if (_bWantDestroy)
	{
		auto it = std::remove(_scene->_gameObjects.begin(), _scene->_gameObjects.end(), this);
		if (it != _scene->_gameObjects.end())
		{
			_scene->_gameObjects.erase(it);
			ExecuteDestroy();
			delete this;
		}
		return false;
	}
	else
	{
		if (_bActive)
		{
			if (!_bInit)//Init
			{
				Init();
			}
			else
			{
				const auto compCount = _comps.size();
				for (int i = 0; i < compCount; i++)
				{
					if (!_comps[i]->Update())
					{
						i -= 1;
					}
				}

			}
		}
	}
	return true;
}

void GameObject::ExecuteDestroy()
{


}
