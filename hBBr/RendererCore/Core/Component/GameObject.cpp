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
	//Create Transform
	_transform = new Transform(this);

	auto sharedPtr = std::shared_ptr<GameObject>(this);
	_selfWeak = sharedPtr;
	_scene->_gameObjects.push_back(sharedPtr);
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

void GameObject::SetParent(GameObject* newParent)
{
	_newParent = newParent;
	_scene->_gameObjectParentSettings.push_back(this);
}

void GameObject::Init()
{
	_bInit = true;
}

bool GameObject::Update()
{
	if (_bWantDestroy)
	{
		auto it = std::find_if(_scene->_gameObjects.begin(), _scene->_gameObjects.end(), [this](std::shared_ptr<GameObject> & obj)
			{
				return obj.get() == this;
			});
		if (it != _scene->_gameObjects.end())
		{	
			//延迟到下一帧再销毁
			_scene->_gameObjectNeedDestroy.push_back(*it);
			_scene->_gameObjects.erase(it);
			ExecuteDestroy();
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
					_comps[i]->Update();
				}
			}
		}
	}
	return true;
}

void GameObject::ExecuteDestroy()
{
	SetParent(NULL);

	while (_comps.size() > 0)
	{
		for (int i = 0; i < _comps.size(); i++)
		{
			_comps[i]->Destroy();
		}
		const auto compCount = _comps.size();
		for (int i = 0; i < compCount; i++)
		{
			if (_comps[i]->_bWantDestroy)
			{
				_comps[i].reset();
				_comps.erase(_comps.begin() + i);
				i -= 1;
				if (_comps.size() <= 0)
					break;
			}
		}
	}
	delete _transform;
	_transform = NULL;
}
