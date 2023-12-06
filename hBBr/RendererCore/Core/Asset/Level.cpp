#include "Asset/Level.h"
#include "Asset/World.h"
#include "FileSystem.h"

Level::Level(HString name)
{
	_levelName = name;
}

Level::~Level()
{
	//wait gameobject destroy
	while (_gameObjects.size() > 0 || _gameObjectNeedDestroy.size() > 0)
	{
		for (int i = 0; i < _gameObjects.size(); i++)
		{
			_gameObjects[i]->Destroy();
		}
		LevelUpdate();
	}
}

void Level::Load(World* world)
{
	if (world)
	{
		_world = world;
	}
}

bool Level::UnLoad()
{
	return false;
}

bool Level::ResetLevel()
{
	if (UnLoad())
	{
		_levelDoc = pugi::xml_document();
		return true;
	}
	return false;
}

void Level::SaveLevel()
{
	if (!_world)
		return;
	HString assetPath = FileSystem::GetWorldAbsPath() + _world->_worldName;
	HString filePath = assetPath + "/" + _world->_worldName + ".world";

}

void Level::LevelUpdate()
{
	//Destroy Objects
	//const auto destroyCount = _gameObjectNeedDestroy.size();
	for (auto& i : _gameObjectNeedDestroy)
	{
		i.reset();
	}
	_gameObjectNeedDestroy.clear();

	//Update Objecets
	for (int i = 0; i < _gameObjects.size(); i++)
	{
		if (!_gameObjects[i]->Update())
		{
			i -= 1;
			if (_gameObjects.size() <= 0)
				break;
		}
		else
		{
#if IS_EDITOR
			if (!_gameObjects[i]->_sceneEditorHide)
				_editorGameObjectUpdateFunc(this, _gameObjects[i]);
#endif
		}
	}

#if IS_EDITOR
	_editorSceneUpdateFunc(this, _gameObjects);
#endif

}

void Level::AddNewObject(std::shared_ptr<GameObject> newObject)
{
	_gameObjects.push_back(newObject);
#if IS_EDITOR
	if (!newObject->_sceneEditorHide)
	{
		if (!newObject->_sceneEditorHide)
			_editorGameObjectAddFunc(this, newObject);
	}
#endif
}

void Level::RemoveObject(GameObject* object)
{
	auto it = std::find_if(_gameObjects.begin(), _gameObjects.end(), [object](std::shared_ptr<GameObject>& obj)
		{
			return obj.get() == object;
		});
	if (it != _gameObjects.end())
	{
		//延迟到下一帧再销毁
		_gameObjectNeedDestroy.push_back(*it);
#if IS_EDITOR
		if (!((*it)->_sceneEditorHide))
			_editorGameObjectRemoveFunc(this, *it);
#endif
		_gameObjects.erase(it);
	}
}

