﻿#include"SceneManager.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "FileSystem.h"
SceneManager::~SceneManager()
{
	//wait gameobject destroy
	while (_gameObjects.size() > 0 || _gameObjectNeedDestroy.size() > 0 || _gameObjectParentSettings.size() > 0 )
	{
		for (int i = 0; i < _gameObjects.size(); i++)
		{
			_gameObjects[i]->Destroy();
		}
		SceneUpdate();
	}
}

void SceneManager::SceneInit(class VulkanRenderer* renderer)
{
	_renderer = renderer;
	//Test
	GameObject* cube = new GameObject();
	auto modelComp = cube->AddComponent<ModelComponent>();
	modelComp->SetModel(FileSystem::GetResourceAbsPath() + "Content/FBX/TestFbx_1_Combine.FBX");
}

void SceneManager::SceneUpdate()
{
	//Setting object parent
	const auto settingObjCount = _gameObjectParentSettings.size();
	for (auto& i : _gameObjectParentSettings)
	{
		if (i->_newParent != NULL)
		{
			if (i->_parent != NULL)
			{
				//为了安全考虑，最好进行一次Parent的查找，不过感觉应该不需要...
				//auto it = std::find(_gameObjects.begin(), _gameObjects.end(), i->_parent);
				//if (it != _gameObjects.end())
				{
					//(*it)->_children.erase();
					//如果已经有父类了先清除
					auto cit = std::find(i->_parent->_children.begin(), i->_parent->_children.end(), i);
					if (cit != i->_parent->_children.end())
					{
						i->_parent->_children.erase(cit);
					}		
				}
			}
			i->_parent = i->_newParent;
			i->_newParent = NULL;
		}
		else
		{
			//如果已经有父类了先清除
			if (i->_parent != NULL)
			{
				auto cit = std::find(i->_parent->_children.begin(), i->_parent->_children.end(), i);
				if (cit != i->_parent->_children.end())
				{
					i->_parent->_children.erase(cit);
				}
			}
			i->_parent = NULL;
		}
	}
	_gameObjectParentSettings.clear();

	//Destroy Objects
	const auto destroyCount = _gameObjectNeedDestroy.size();
	for (auto& i : _gameObjectNeedDestroy)
	{
		delete i;
	}
	_gameObjectNeedDestroy.clear();

	//Update Objecets
	const auto objCount = _gameObjects.size();
	for (int i = 0; i < objCount ; i++)
	{
		if (!_gameObjects[i]->Update())
		{
			i -= 1;
			if (_gameObjects.size() <= 0)
				break;
		}
	}
}