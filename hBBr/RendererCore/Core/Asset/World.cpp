#include"World.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Component/CameraComponent.h"
#include "FileSystem.h"
#include "ConsoleDebug.h"
#include "FormMain.h"
std::vector<std::weak_ptr<World>>World::_dirtyWorlds;
World::World()
{

}

World::~World()
{
	ReleaseWorld();
	ConsoleDebug::printf_endl("Release World[%s][%s]", this->GetWorldName().c_str(), this->GetGUID().str().c_str());
}

Level* World::GetLevel(HString name)
{
	auto it = std::find_if(_levels.begin(), _levels.end(), [&](std::shared_ptr<Level>& level) {
		return level->GetLevelName().IsSame(name);
	});
	if (it != _levels.end())
	{
		return it->get();
	}
	return nullptr;
}

void World::AddNewLevel(HString name)
{
	std::shared_ptr<Level> newLevel = nullptr;
	newLevel.reset(new Level(this, name));
	newLevel->Load();
	_levels.push_back(newLevel);
	newLevel->MarkDirty();
#if IS_EDITOR
	_editorLevelChanged();
	MarkDirty();
#endif
}

void World::DeleteLevel(HString levelName)
{
	//此操作并不会删除.level文件，只会更改World的_levels，等待DirtyAssetsManager里确认保存了才会真正删除保存

	//移除World里面的内存，保存的时候会根据_levels重新写入json
	auto it = std::remove_if(_levels.begin(), _levels.end(), 
		[&](std::shared_ptr<Level>& l) {
			return l->GetLevelName().IsSame(levelName);
		});
	if (it != _levels.end())
	{
		it->get()->DeleteLevel();
		_levels.erase(it);
		//标记需要保存
		#if IS_EDITOR
			MarkDirty();
		#endif
	}
}

std::shared_ptr<World> World::CreateNewWorld(HString newWorldName)
{
	std::shared_ptr<World> world;
	world.reset(new World());

	HString finalName = newWorldName;
	/*auto it = _worlds.find(finalName);
	int index = 0;
	while (it != _worlds.end())
	{
		finalName = newWorldName + "_" +  HString::FromInt(index);
		it = _worlds.find(finalName);
		index++;
	}*/

	world->_guid = CreateGUID();
	world->_guidStr = world->_guid.str();
	world->_worldName = finalName;
	world->_worldAbsPath = FileSystem::Append(FileSystem::GetWorldAbsPath(), world->_guidStr) + ".world";
	world->_worldSettingAbsPath = FileSystem::Append(FileSystem::GetWorldAbsPath(), ".WorldSettings");
	FileSystem::FixUpPath(world->_worldSettingAbsPath);
	ConsoleDebug::printf_endl("Create new world[%s][%s]", finalName.c_str(), world->_guidStr.c_str());
	return world;
}

void World::SetWorldName(HString name)
{
	_worldName = name;
	ConsoleDebug::printf_endl("World rename[%s][%s]", _worldName.c_str(), _guidStr.c_str());
}

void World::SaveWorld(HString newWorldName)
{
	newWorldName.ClearSpace();
	//创建World目录
	FileSystem::CreateDir(_worldAbsPath.c_str());
	for (auto& i : _levels)
	{
		i->SaveLevel();
	}
	//World Setting
	SaveWorldSetting();
}

void World::SaveWorldSetting()
{
	ToJson();
	SaveJson(_worldSettingAbsPath);
}

void World::ReloadWorldSetting()
{
	if (FileSystem::FileExist(_worldSettingAbsPath))
	{
		if (LoadJson(_worldSettingAbsPath, _json))
		{
			_worldName = _json["WorldName"];
			nlohmann::json levels = _json["Levels"];
			for (auto& i : levels.items())
			{
				HString levelName = i.key();
				bool bVisibility = i.value()["Visibility"];
				std::shared_ptr<Level> newLevel;

				auto level_it = std::find_if(_levels.begin(), _levels.end(), [levelName](std::shared_ptr<Level>& l) {
					return l->GetLevelName() == levelName;
				});
				if (level_it != _levels.end())
				{
					level_it->get()->_bInitVisibility = bVisibility;
					level_it->get()->Rename(levelName);
				}
				else
				{
					newLevel.reset(new Level(this, levelName));
					newLevel->_bInitVisibility = bVisibility;
					_levels.push_back(newLevel);
				}
				from_json(i.value()["GUID"], newLevel->_guid);
				auto deps_it = i.value().find("Dep");
				if (deps_it != i.value().end())
				{
					for (auto& i : deps_it.value().items())
					{
						newLevel->_dependency.emplace(i.key(), (AssetType)i.value()["Type"]);
					}
				}
			}
			#if IS_EDITOR
				_editorLevelChanged();
			#endif
		}
	}
}

GameObject* World::SpawnGameObject(HString name, class Level* level)
{
#if IS_EDITOR
	if (level != _editorLevel.get() && this->_levels.size() <= 0)
#else
	if (this->_levels.size() <= 0)
#endif
	{
		MsgBox("World.cpp/SpawnGameObject","Spawn game object failed.This world is not having any levels.");
		return nullptr;
	}
	if (!level)
	{
		level = this->_levels[0].get();
	}
	if (!level->_bLoad)
	{
		MsgBox("World.cpp/SpawnGameObject", "Spawn game object failed.The level is not loading.");
		return nullptr;
	}
	GameObject* newObject = GameObject::CreateGameObject(name, level);
	return newObject;
}

void World::Load(class VulkanRenderer* renderer)
{
	_renderer = renderer;

#if IS_EDITOR
	_editorLevelChanged();
#endif

#if IS_EDITOR

	//Create editor only level.
	_editorLevel.reset(new Level(this, "EditorLevel"));
	_editorLevel->_isEditorLevel = true;
	_editorLevel->Load();

	//create editor camera
	auto backCamera = GameObject::CreateGameObject("EditorCamera", _editorLevel.get());
	backCamera->_sceneEditorHide = true;
	
	glm::vec3 camPos = glm::vec3(0, 2, -3.0);
	glm::vec3 camRot = glm::vec3(0);
	if(_renderer->IsMainRenderer())
	{
		auto it = _json.find("DefaultCameraPosition");
		if (it != _json.end())
		{
			from_json(it.value(), camPos);
		}
	}
	if(_renderer->IsMainRenderer())
	{
		auto it = _json.find("DefaultCameraRotation");
		if (it != _json.end())
		{
			from_json(it.value(), camRot);
		}
	}
	backCamera->GetTransform()->SetWorldLocation(camPos);
	backCamera->GetTransform()->SetWorldRotation(camRot);
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

	_bLoad = true;

	for (auto& i : _levels)
	{
		if (i->_bInitVisibility)
		{
			i->Load();
		}
	}

#if IS_GAME
	//-----model--camera
	auto backCamera = GameObject::CreateGameObject("TestGameCamera", _levels[0].get());
	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
	auto cameraComp = backCamera->AddComponent<CameraComponent>();
	cameraComp->OverrideMainCamera();
#endif

	////-----model--test
	//auto testModel = GameObject::CreateGameObject("Test", _levels[0].get());
	//auto modelComp = testModel->AddComponent<ModelComponent>();
	//modelComp->SetModel(HGUID("c51a01e8-9349-660a-d2df-353a310db461"));
	//ConsoleDebug::printf_endl("Test Model Spawn......");
}

bool World::ReleaseWorld()
{
	for (auto& i : _levels)
	{
		i.reset();
	}
#if IS_EDITOR
	_editorLevel.reset();
#endif
	_levels.clear();
	return true;
}

void World::WorldUpdate()
{
	std::vector < std::weak_ptr<Level> >_levelPtrs;
	_levelPtrs.resize(_levels.size() + 1);
	if (!_bLoad)
	{
		return;
	}
	for (int i = 0; i < _levels.size(); i++)
	{
		_levelPtrs[i] = _levels[i];
		_levels[i]->LevelUpdate();
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

void World::MarkDirty()
{
		AddDirtyWorld(this->_renderer->GetWorld());
}

#endif

nlohmann::json World::ToJson()
{
	//Level
	nlohmann::json levels;
	for (int i = 0; i < _levels.size(); i++)
	{
		nlohmann::json subLevel;
		subLevel["Visibility"] = _levels[i]->_bLoad;
		subLevel["GUID"] = _levels[i]->GetGUID();
		//dep依赖
		nlohmann::json levelDeps;
		for (auto& i : _levels[i]->_dependency)
		{
			levelDeps[i.first.str()]["Type"] = (int)i.second;
		}
		subLevel["Dep"] = levelDeps;
		levels[_levels[i]->GetLevelName().c_str()] = subLevel;
	}

	//World
	_json["WorldName"] = _worldName.c_str();
	_json["Levels"] = levels;
	//cam
	if (_mainCamera)
	{
		to_json(_json["DefaultCameraPosition"], _mainCamera->GetTransform()->GetWorldLocation());
		to_json(_json["DefaultCameraRotation"], _mainCamera->GetTransform()->GetWorldRotation());
	}

	return _json;
}

void World::FromJson()
{
}