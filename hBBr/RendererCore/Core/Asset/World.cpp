#include"World.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Component/CameraComponent.h"
#include "FileSystem.h"
#include "XMLStream.h"
#include "HInput.h"

World::World(class VulkanRenderer* renderer)
{
	Load(renderer, "");
}

World::World(VulkanRenderer* renderer, HString worldAssetPath)
{
	Load(renderer, worldAssetPath);
}

World::~World()
{
	ReleaseWorld();
}

void World::AddLevel(HString levelNameOrAssetPath)
{
	HString path = levelNameOrAssetPath; 
	//Asset完整路径
	if (FileSystem::FileExist(path))
	{

	}
	else
	{
		//Asset相对路径
		path = FileSystem::FillUpAssetPath(levelNameOrAssetPath);
		if (FileSystem::FileExist(path))
		{

		}
		//Name
		else
		{
			path = FileSystem::Append(_worldAssetPath, levelNameOrAssetPath) + ".level";
			if (FileSystem::FileExist(path))
			{

			}
		}
	}
	_editorLevelChanged();
}

void World::AddNewLevel(HString name)
{
	std::shared_ptr<Level> newLevel = nullptr;
	newLevel.reset(new Level(name));
	newLevel->Load(this, "");
	_levels.push_back(newLevel);
	_editorLevelChanged();
}

void World::SaveWorld(HString newWorldName)
{
	newWorldName.ClearSpace();
	if (newWorldName.Length() > 1)
	{
		HString assetPath = FileSystem::GetWorldAbsPath();
		newWorldName = assetPath + "/" + newWorldName + ".world";
		FileSystem::FixUpPath(newWorldName);
		_worldAssetPath = newWorldName;
	}
	//创建World目录
	FileSystem::CreateDir(_worldAssetPath.c_str());
	for (auto& i : _levels)
	{
		i->SaveLevel();
	}
}

GameObject* World::SpawnGameObject(HString name, class Level* level)
{
	if (this->_levels.size() <= 0)
	{
		MsgBox("World.cpp/SpawnGameObject","Spawn game object failed.This world is not having any levels.");
		return nullptr;
	}
	if (!level)
	{
		level = this->_levels[0].get();
	}
	if (!level->bLoad)
	{
		MsgBox("World.cpp/SpawnGameObject", "Spawn game object failed.The level is not loading.");
		return nullptr;
	}
	GameObject* newObject = GameObject::CreateGameObject(name, level);
	newObject->SetObjectName(name);
	return newObject;
}

void World::Load(class VulkanRenderer* renderer, HString worldAssetPath)
{
	_renderer = renderer;

	HString assetPath = FileSystem::GetWorldAbsPath();

	if (!FileSystem::FileExist(assetPath))
	{
		FileSystem::CreateDir(assetPath.c_str());
	}

	HString dirPath = assetPath + "/" + _worldName + ".world";
	FileSystem::FixUpPath(dirPath);
	_worldAssetPath = dirPath;

#if IS_EDITOR

	//Create editor only level.
	_editorLevel.reset(new Level("EditorLevel"));
	_editorLevel->Load(this, "");
	_editorLevel->_isEditorLevel = true;

	//create editor camera
	auto backCamera = GameObject::CreateGameObject("EditorCamera", _editorLevel.get());
	backCamera->_sceneEditorHide = true;
	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
	auto cameraComp = backCamera->AddComponent<CameraComponent>();
	cameraComp->OverrideMainCamera();
	_editorCamera = cameraComp;
	_editorCamera->_bIsEditorCamera = true;

#else

//	//Test game camera
//	auto backCamera = new GameObject("Camera");
//	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
//	auto cameraComp = backCamera->AddComponent<CameraComponent>();
//	cameraComp->OverrideMainCamera();

#endif

	//Load world content
	if (FileSystem::IsDir(worldAssetPath.c_str()))
	{
		//Find all levels
		auto levelFiles = FileSystem::GetFilesBySuffix(worldAssetPath.c_str(), "level");
		for (auto f : levelFiles)
		{
			AddLevel(f.relativePath);
		}
	}
	else
	{
		//This is a new world ,create a empty level .
		AddNewLevel("Empty Level");
	}
	bLoad = true;
}

bool World::ReleaseWorld()
{
	for (auto& i : _levels)
	{
		i.reset();
	}
	_editorLevel.reset();
	_levels.clear();
	return true;
}

void World::WorldUpdate()
{
	std::vector < std::weak_ptr<Level> >_levelPtrs;
	_levelPtrs.resize(_levels.size() + 1);
	if (!bLoad)
	{
		return;
	}
	for (int i = 0; i < _levels.size(); i++)
	{
		_levelPtrs[i] = _levels[i];
		if (_levels[i]->bLoad)
		{
			_levels[i]->LevelUpdate();
		}
	}

	//Update Editor if the function is not null.
#if IS_EDITOR
	if (_editorLevel)
	{
		_editorLevel->LevelUpdate();
		_levelPtrs[_levelPtrs.size() - 1] = _editorLevel;
	}
	_editorWorldUpdate(_levelPtrs);
#endif

}

void World::UpdateObject(std::shared_ptr<GameObject> newObject)
{
#if IS_EDITOR
	if (!newObject->_sceneEditorHide)
		_editorGameObjectUpdateFunc(newObject);
#endif
}

void World::AddNewObject(std::shared_ptr<GameObject> newObject)
{
#if IS_EDITOR
	if (!newObject->_sceneEditorHide)
	{
		_editorGameObjectAddFunc(newObject);
	}
#endif
}

void World::RemoveObject(std::shared_ptr<GameObject> object)
{
#if IS_EDITOR
	if (!(object->_sceneEditorHide))
		_editorGameObjectRemoveFunc(object);
#endif
}

#if IS_EDITOR

void World::SetCurrentSelectionLevel(std::weak_ptr<Level> level)
{
	_currentSelectionLevel = level;
}

#endif