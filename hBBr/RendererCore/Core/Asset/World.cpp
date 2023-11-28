#include"World.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Component/CameraComponent.h"
#include "FileSystem.h"
#include "XMLStream.h"
#include "HInput.h"

std::weak_ptr<World> World::LoadAsset(HGUID guid)
{
	const auto assets = ContentManager::Get()->GetAssets(AssetType::World);
	HString guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = assets.find(guid);
	{
		if (it == assets.end())
		{
			MessageOut(HString("Can not find [" + guidStr + "] world in content manager.").c_str(), false, false, "255,255,0");
			return std::weak_ptr<World>();
		}
	}
	auto dataPtr = reinterpret_cast<AssetInfo<World>*>(it->second);
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	//获取实际路径
	HString filePath = FileSystem::GetProgramPath() + it->second->relativePath + guidStr + ".mat";
	FileSystem::CorrectionPath(filePath);
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return std::weak_ptr<World>();
	}



	return std::weak_ptr<World>();
}

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

//void World::SaveWorld(HString path)
//{
//	if (FileSystem::IsDir(path.c_str()))
//	{
//		HString filePath = path + _worldName + ".xml";
//		pugi::xml_document doc;
//		XMLStream::CreateXMLFile("", doc);
//		auto root = doc.append_child(L"root");
//		//save scenes
//		auto scene = root.append_child(L"scene");
//		for (auto& i : _sceneDocs)
//		{
//			HString name = i.first;
//			HString guidStr = i.second.str().c_str();
//			auto item = scene.append_child(L"Item");
//			item.append_attribute(L"Name").set_value(name.c_wstr());
//			item.append_attribute(L"GUID").set_value(guidStr.c_wstr());
//		}
//		doc.save_file(filePath.c_wstr());
//	}
//	else
//	{
//		MessageOut("[SaveWorld] function parameter 'path' is not a valid directory.", false, false, "255,255,0");
//	}
//}

void World::AddLevel(HString levelNameOrContentPath)
{
	HGUID guid;
	StringToGUID(levelNameOrContentPath.c_str(), &guid);
	AssetInfo<Level>* newLevel = NULL;
	if (guid.isValid())
	{
		auto asset  = ContentManager::Get()->GetAssetInfo(guid, AssetType::Level);
		if (asset)
			newLevel = (AssetInfo<Level>*)asset;
	}
	else
	{
		auto asset = ContentManager::Get()->GetAssetInfo(levelNameOrContentPath);
		if(asset)
			newLevel = (AssetInfo<Level>*)asset;
		else
		{
			auto asset = ContentManager::Get()->GetAssetInfo(AssetType::Level, levelNameOrContentPath);
			if (asset)
				newLevel = (AssetInfo<Level>*)asset;
		}
	}
	if (!newLevel)
	{
		#if IS_EDITOR
				MessageOut(HString("Add Level Failed.").c_str(), true, false, "255,0,0");
		#else
				MessageOut(HString("Add Level Failed.").c_str(), false, false, "255,0,0");
		#endif
		return;
	}
	else
	{
		_levels.push_back(newLevel->GetData());
	}
}

void World::AddLevel(HGUID guid)
{
	AssetInfo<Level>* newLevel = NULL;
	if (guid.isValid())
	{
		auto asset = ContentManager::Get()->GetAssetInfo(guid, AssetType::Level);
		if (asset)
			newLevel = (AssetInfo<Level>*)asset;
	}
	if (!newLevel)
	{
		#if IS_EDITOR
				MessageOut(HString("Add Level Failed.").c_str(), true, false, "255,0,0");
		#else
				MessageOut(HString("Add Level Failed.").c_str(), false, false, "255,0,0");
		#endif
		return;
	}
	else
	{
		_levels.push_back(newLevel->GetData());
	}
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

}

bool World::UnLoad()
{
	_refCount--;
	//引用计数等于0了才能卸载
	if (_refCount == 0)
	{
		return true;
	}
	return false;
}

bool World::ReleaseWorld()
{
	if (UnLoad())
	{
		((AssetInfo<World>*)this->_assetInfo)->ReleaseData();
		return true;
	}
	return false;
}

void World::WorldUpdate()
{
	for (int i = 0; i < _levels.size(); i++)
	{
		if (_levels[i].expired())
		{
			_levels.erase(_levels.begin() + i);
			i--;
			continue;
		}
		else
		{
			if (_levels[i].lock()->bLoad)
			{
				_levels[i].lock()->LevelUpdate();
			}			
		}	
	}
	////Test
	//if (!testObj.expired() && testObj.lock()->GetTransform())
	//{
	//	testObj.lock()->GetTransform()->SetRotation(testObj.lock()->GetTransform()->GetRotation() + glm::vec3(0, _renderer->GetFrameRate() / 10.0f, 0));
	//}

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
