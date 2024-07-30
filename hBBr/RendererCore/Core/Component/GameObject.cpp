#include"GameObject.h"
#include "FormMain.h"
#include "VulkanRenderer.h"
#include "Asset/World.h"
#include "Asset/Level.h"
#include "Component.h"
#include "ConsoleDebug.h"
#include "Component/ModelComponent.h"

GameObject::GameObject(HString objectName, Level* level, bool SceneEditorHide)
{
	ObjectInit(objectName, level, SceneEditorHide);
}

GameObject::GameObject(Level* level, bool SceneEditorHide)
{
	ObjectInit("NewGameObject", level, SceneEditorHide);
}

GameObject::GameObject(HString objectName, HString guidStr, Level* level)
{
	_guidStr = guidStr;
	StringToGUID(guidStr.c_str(), &_guid);
	ObjectInit(objectName, level);
}

GameObject::~GameObject()
{

}

void GameObject::ObjectInit(HString objectName, Level* level, bool SceneEditorHide)
{
	bool FromXmlNode = false;
	if (!_guid.isValid())
	{
		_guidStr = CreateGUIDAndString(_guid);
	}
	else
	{
		FromXmlNode = true;//GUID已知，是从scene xml里获取到的
	}
#if IS_EDITOR
	_sceneEditorHide = SceneEditorHide;
#endif
	if (level == nullptr)
	{
		_world = VulkanApp::GetMainForm()->renderer->GetWorld().lock().get();
		_level = _world->_levels[0].get();
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

	//auto sharedPtr = std::shared_ptr<GameObject>(this);
	//_selfWeak = sharedPtr;
	//_level->AddNewObject(sharedPtr);
}

GameObject* GameObject::CreateGameObject(HString objectName, Level* level)
{
	std::shared_ptr<GameObject> sharedPtr;
	sharedPtr.reset(new GameObject(objectName, level));
	sharedPtr->_selfWeak = sharedPtr;
	level->AddNewObject(sharedPtr);
	return sharedPtr.get();
}

GameObject* GameObject::CreateGameObjectWithGUID(HString objectName, HString guidStr, Level* level)
{
	std::shared_ptr<GameObject> sharedPtr;
	sharedPtr.reset(new GameObject(objectName, guidStr, level));
	sharedPtr->_selfWeak = sharedPtr;
	level->AddNewObject(sharedPtr);
	return sharedPtr.get();
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
	if (newParent != nullptr)
	{
		if (_parent != nullptr)
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
		if (_parent->_children.capacity() <= _parent->_children.size())
		{
			_parent->_children.reserve(_parent->_children.capacity() + 10);
		}
		_parent->_children.push_back(this);
		if (_transform)
			_transform->ResetTransformForAttachment();
		#if IS_EDITOR
		ConsoleDebug::print_endl("GameObject " + _name + " attach to  : " + newParent->GetObjectName());
		#endif

		_level->GetWorld()->_editorGameObjectSetParentFunc(this->_selfWeak.lock(),this->_parent->_selfWeak.lock());
	}
	else
	{
		//如果已经有父类了先清除
		if (_parent != nullptr)
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
		_parent = nullptr;

		_level->GetWorld()->_editorGameObjectSetParentFunc(this->_selfWeak.lock(), nullptr);
	}

}

void GameObject::ChangeLevel(HString newLevel)
{
	auto level = this->_world->GetLevel(newLevel);
	auto oldLevel = _level;
	if (level)
	{
		//移动到新的Level需要加载,才能正常保存
		level->Load();
		//把共享指针移动到新的Level
		level->AddNewObject(this->_selfWeak.lock(), true);
		//移动到新Level，删除Parent
		SetParent(nullptr);
		//设置新的level
		_level = level;
		//移除旧Level内的共享指针
		oldLevel->RemoveObject(this, true);
		#if IS_EDITOR
		level->MarkDirty();
		oldLevel->MarkDirty();
		#endif
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
				if(_transform != nullptr)
					_transform->Update();
				const auto compCount = _comps.size();
				for (int i = 0; i < compCount; i++)
				{
					_comps[i]->CompUpdate();
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
		auto it = std::remove_if(_comps.begin(), _comps.end(), [&](Component*& comp) {
			return comp == _comps[i];
			});
		if (it != _comps.end())
		{
			delete _comps[i];
			_comps[i] = nullptr;
			_comps.erase(it);
		}
	}

	if (_transform != nullptr)
	{
		delete _transform;
		_transform = nullptr;
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
