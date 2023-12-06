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
	Load(renderer);
}

World::~World()
{
	ReleaseWorld();
}

void World::AddNewLevel(HString name)
{
	std::shared_ptr<Level> newLevel = NULL;
	newLevel.reset(new Level(name));
	newLevel->Load(this);
	_levels.push_back(newLevel);
}

void World::SaveWorld()
{
	HString assetPath = FileSystem::GetWorldAbsPath() + _worldName ;
	HString filePath = assetPath + "/" + _worldName + ".world";
	//创建World目录
	FileSystem::CreateDir(filePath.c_str());
}

void World::SaveWholeWorld()
{
	SaveWorld();
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
		return NULL;
	}
	if (!level)
	{
		level = this->_levels[0].get();
	}
	GameObject* newObject = new GameObject(level);
	newObject->SetObjectName(name);
	return newObject;
}

void World::Load(class VulkanRenderer* renderer)
{
	_renderer = renderer;

#if IS_EDITOR

	//Create editor only level.
	_editorLevel.reset(new Level("EditorLevel"));
	_editorLevel->Load(this);
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
//
//	//Test model
//	auto test = new GameObject(this);
//	auto modelComp0 = test->AddComponent<ModelComponent>();
//	modelComp0->SetModelByVirtualPath(FileSystem::GetAssetAbsPath() + "Content/Core/Basic/TestFbx_1_Combine");
//	test->SetObjectName("TestFbx_1_Combine");
//
//	GameObject* cube = new GameObject(this);
//	testObj = cube->GetSelfWeekPtr();
//	auto modelComp = cube->AddComponent<ModelComponent>();
//	cube->GetTransform()->SetLocation(glm::vec3(0, 0.5f, 0));
//	modelComp->SetModelByVirtualPath(FileSystem::GetAssetAbsPath() + "Content/Core/Basic/Cube");
//	cube->SetObjectName("TestFbx_Cube");

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