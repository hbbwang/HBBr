#include"GameObject.h"
#include "FormMain.h"
#include "VulkanRenderer.h"
#include "Asset/World.h"
#include "Asset/Level.h"
#include "Component.h"
#include "ConsoleDebug.h"
#include "Component/ModelComponent.h"

std::map<HString, std::function<class Component* (class GameObject*)>> GameObject::_componentSpawnFunctions;

GameObject::GameObject(HString objectName, Level* level, bool SceneEditorHide)
{
	ObjectInit(objectName, level, SceneEditorHide);
}

GameObject::GameObject(Level* level, bool SceneEditorHide)
{
	ObjectInit("NewGameObject", level, SceneEditorHide);
}

GameObject::~GameObject()
{

}

void GameObject::ObjectInit(HString objectName, Level* level, bool SceneEditorHide)
{
	_guid = CreateGUID();
#if IS_EDITOR
	_sceneEditorHide = SceneEditorHide;
#endif
	if (level == NULL)
	{
		_world = VulkanApp::GetMainForm()->renderer->GetWorld();
		_level = VulkanApp::GetMainForm()->renderer->GetWorld()->_levels[0].get();
	}
	else
	{
		_world = level->GetWorld();
		_level = level;
#if IS_EDITOR
		_IsEditorObject = _level->_isEditorLevel;
#endif
	}
	_name = objectName;
	_bActive = true;
	//Create Transform
	_transform = new Transform(this);

	auto sharedPtr = std::shared_ptr<GameObject>(this);
	_selfWeak = sharedPtr;
	_level->AddNewObject(sharedPtr);
}

GameObject* GameObject::CreateGameObject(HString objectName, Level* level)
{
	return new GameObject(objectName, level);
}

GameObject* GameObject::CreateModelGameObject(HString virtualPath, Level* level)
{
	GameObject* cube = new GameObject(virtualPath.GetBaseName(), level);
	auto modelComp = cube->AddComponent<ModelComponent>();
	cube->GetTransform()->SetLocation(glm::vec3(0, 0.5f, 0));
	modelComp->SetModelByVirtualPath(virtualPath);
	return cube;
}

void GameObject::SetActive(bool newActive)
{
	_bActive = newActive;
	for (auto c : _comps)
	{
		c->GameObjectActiveChanged(_bActive);
	}
	if (!newActive)//When object disable,what's going to happen?
	{
	}
}

void GameObject::SetObjectName(HString newName)
{
#if IS_EDITOR
	ConsoleDebug::print_endl("GameObject "+ _name +" rename : " + newName);
	if(!_sceneEditorHide)
		_bEditorNeedUpdate = true;
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
		_parent->_children.push_back(this);
		if (_transform)
			_transform->ResetTransformForAttachment();
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

			if (_transform)
				_transform->ResetTransformForAttachment();
			#if IS_EDITOR
			ConsoleDebug::print_endl("GameObject " + _name + " detach");
			#endif
		}
		_parent = NULL;
	}

}

void GameObject::Init()
{
	#if IS_EDITOR
	if (!_sceneEditorHide)
		_bEditorNeedUpdate = true;
	#endif
	_bInit = true;
}

bool GameObject::Update()
{
	if (_bWantDestroy)
	{
		if (ExecuteDestroy())
		{
			_level->RemoveObject(this);
			return false;
		}
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

bool GameObject::ExecuteDestroy()
{
	if (_children.size() > 0)
	{
		for (auto i : _children)
		{
			i->Destroy();
		}
		_children.clear();
		return false;
	}

	for (int i = 0; i < _comps.size(); i++)
	{
		_comps[i]->Destroy();
	}

	if (_transform != NULL)
	{
		delete _transform;
		_transform = NULL;
	}

	if (_comps.size() > 0)
		return false;
	else
	{
		#if IS_EDITOR
		ConsoleDebug::print_endl("GameObject " + _name + " has been Destroy.");
		#endif
		return true;
	}
}
