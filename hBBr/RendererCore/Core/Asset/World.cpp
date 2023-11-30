#include"World.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Component/CameraComponent.h"
#include "FileSystem.h"
#include "XMLStream.h"
#include "HInput.h"

World::~World()
{
	//wait gameobject destroy
	while (_gameObjects.size() > 0 || _gameObjectNeedDestroy.size() > 0)
	{
		for (int i = 0; i < _gameObjects.size(); i++)
		{
			_gameObjects[i]->Destroy();
		}
		WorldUpdate();
	}
}

void World::AddNewLevel(HString name)
{
	std::shared_ptr<Level> newLevel = NULL;
	newLevel.reset(new Level(name));
	_levels.push_back(newLevel);
}

void World::SaveWorld()
{
	HString assetPath = FileSystem::GetWorldAbsPath() + _worldName ;
	HString filePath = assetPath + "/" + _worldName + ".world";
	if (!FileSystem::FileExist(assetPath.c_str()) && !FileSystem::FileExist(filePath.c_str()))
	{
		pugi::xml_document doc;	
		if (!XMLStream::LoadXML(filePath.c_wstr(), doc))
		{
			XMLStream::CreateXMLFile(filePath, doc);
		}
		auto root = doc.append_child(L"root");
		//save scenes
		auto scene = root.append_child(L"Level");
		scene.append_attribute(L"Num").set_value((int)_levels.size());
		for (auto& i : _levels)
		{
			HString name = i->_levelName;
			auto item = scene.append_child(L"Item");
			item.append_attribute(L"Name").set_value(name.c_wstr());
		}
		doc.save_file(filePath.c_wstr());
	}
	else
	{
		MessageOut("[SaveWorld] function parameter 'path' is not a valid directory.", false, false, "255,255,0");
	}
}

void World::SaveWholeWorld()
{
	for (auto& i : _levels)
	{
		i->SaveLevel();
	}
	SaveWorld();
}

void World::Load(class VulkanRenderer* renderer)
{
	_renderer = renderer;
#if IS_EDITOR
	//create editor camera
	auto backCamera = new GameObject("EditorCamera", this, true);
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

}

bool World::UnLoad()
{
	bLoad = false;
	return true;
}

bool World::ReleaseWorld()
{
	if (UnLoad())
	{
		return true;
	}
	return false;
}

void World::WorldUpdate()
{
	if (!bLoad)
	{
		return;
	}
	for (int i = 0; i < _levels.size(); i++)
	{
		if (_levels[i]->bLoad)
		{
			_levels[i]->LevelUpdate();
		}
	}

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

	//Update Editor if the function is not null.
	#if IS_EDITOR
	_editorSceneUpdateFunc(this, _gameObjects);
	#endif
}

void World::AddNewObject(std::shared_ptr<GameObject> newObject)
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

void World::RemoveObject(GameObject* object)
{
	auto it = std::find_if(_gameObjects.begin(),_gameObjects.end(), [object](std::shared_ptr<GameObject>& obj)
		{
			return obj.get() == object;
		});
	if (it != _gameObjects.end())
	{
		//延迟到下一帧再销毁
		_gameObjectNeedDestroy.push_back(*it);
		#if IS_EDITOR
		if(!((*it)->_sceneEditorHide))
			_editorGameObjectRemoveFunc(this, *it);
		#endif
		_gameObjects.erase(it);
	}
}
