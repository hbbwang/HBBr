#include"GameObject.h"
#include "FormMain.h"
#include "VulkanRenderer.h"
#include "Resource/SceneManager.h"
#include "Component.h"
#include "ConsoleDebug.h"
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
	_scene->AddNewObject(sharedPtr);
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

void GameObject::SetObjectName(HString newName)
{
#if IS_EDITOR
	ConsoleDebug::print_endl("GameObject "+ _name +" rename : " + newName);
#endif
	_name = newName;
}

void GameObject::SetParent(GameObject* newParent)
{
	if (newParent != NULL)
	{
		if (_parent != NULL)
		{
			//为了安全考虑，最好进行一次Parent的查找，不过感觉应该不需要...
			//auto it = std::find(_gameObjects.begin(), _gameObjects.end(), i->_parent);
			//if (it != _gameObjects.end())
			{
				//(*it)->_children.erase();
				//如果已经有父类了先清除
				auto cit = std::find(_parent->_children.begin(), _parent->_children.end(), this);
				if (cit != _parent->_children.end())
				{
					_parent->_children.erase(cit);
				}
			}
		}
		_parent = newParent;
#if IS_EDITOR
		ConsoleDebug::print_endl("GameObject " + _name + " attach to  : " + newParent->GetObjectName());
#endif
	}
	else
	{
		//如果已经有父类了先清除
		if (_parent != NULL)
		{
			auto cit = std::find(_parent->_children.begin(), _parent->_children.end(), this);
			if (cit != _parent->_children.end())
			{
				_parent->_children.erase(cit);
			}
		}
		_parent = NULL;
#if IS_EDITOR
		ConsoleDebug::print_endl("GameObject " + _name + " detach");
#endif
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
		_scene->RemoveObject(this);
		ExecuteDestroy();
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
				if(_transform != NULL)
					_transform->Update();
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
