#include"World.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Component/CameraComponent.h"
#include "FileSystem.h"
#include "XMLStream.h"
#include "HInput.h"

#if IS_EDITOR
std::map<HGUID, std::function<void(VulkanRenderer*, World*)>> World::_editorSpwanNewWorld;

#endif

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

void World::AddLevel(HString levelAssetPath)
{
	if (FileSystem::ContainsPath(_worldAssetPath, levelAssetPath))
	{

	}
}

void World::AddNewLevel(HString name)
{
	std::shared_ptr<Level> newLevel = nullptr;
	newLevel.reset(new Level(name));
	newLevel->Load(this, "");
	_levels.push_back(newLevel);
}

void World::SaveWorld(HString newWorldName)
{
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
	GameObject* newObject = new GameObject(level);
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
	auto backCamera = new GameObject("EditorCamera", _editorLevel.get(), true);
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

	//Load world file
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
#if IS_EDITOR
	for (auto i : World::_editorSpwanNewWorld)
	{
		i.second(_renderer, this);
	}
#endif
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
	std::vector < Level* >_levelPtrs;
	_levelPtrs.resize(_levels.size());
	if (!bLoad)
	{
		return;
	}
	for (int i = 0; i < _levels.size(); i++)
	{
		_levelPtrs[i] = _levels[i].get();
		if (_levels[i]->bLoad)
		{
			_levels[i]->LevelUpdate();
		}
	}

	//Update Editor if the function is not null.
#if IS_EDITOR
	if(_editorLevel) _editorLevel->LevelUpdate();
	for (auto i : _editorWorldUpdate)
	{
		i.second(this, _levelPtrs);
	}
#endif

}

#if IS_EDITOR

void World::SetCurrentSelectionLevel(std::weak_ptr<Level> level)
{
	_currentSelectionLevel = level;
}

#endif