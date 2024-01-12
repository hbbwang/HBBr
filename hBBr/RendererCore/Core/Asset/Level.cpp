#include "Asset/Level.h"
#include "Asset/World.h"
#include "FileSystem.h"
#include "XMLStream.h"
#include "Component/Component.h"

Level::Level(HString name)
{
	_levelName = name;
	_levelName = _levelName.ClearSpace();
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

void Level::Rename(HString newName)
{
	_levelName = newName;
	if (FileSystem::FileExist(_levelPath))
	{
		FileSystem::FileRemove(_levelPath.c_str());
		SaveLevel();
	}
#if IS_EDITOR
	_world->_editorLevelChanged();
#endif
}

GameObject* Level::FindGameObjectByGUID(HGUID guid)
{
	auto it = std::find_if(_gameObjects.begin(), _gameObjects.end(), [guid](std::shared_ptr<GameObject>& item) {
		return item->GetGUID() == guid;
	});
	if (it != _gameObjects.end())
	{
		return it->get();
	}
	return nullptr;
}

void Level::Load(World* world, HString levelPath)
{
	if (world)
	{
		_world = world;
		if (!bLoad)
		{
			XMLStream::LoadXML(levelPath.c_wstr(), _levelDoc);
			if (_levelDoc && !_levelDoc.empty())
			{
				//先生成
				auto root = _levelDoc.child(L"root");
				HString type;
				for (pugi::xml_node item = root.first_child(); item; item = item.next_sibling())
				{
					HString type = item.attribute(L"Type").as_string();					
					if (type == "GameObject")
					{
						HString name = item.attribute(L"Name").as_string();
						auto object = _world->SpawnGameObject(name, this);
					}
				}
				//再考虑父子关系
				uint32_t objectIndex = 0;
				for (pugi::xml_node item = root.first_child(); item; item = item.next_sibling())
				{
					if (type == "GameObject")
					{
						HString guidStr = item.attribute(L"GUID").as_string();
						HGUID  guid; StringToGUID(guidStr.c_str(), &guid);
						auto object = _gameObjects[objectIndex];
						//Parent
						HString parentGuidStr = item.attribute(L"Parent").as_string();
						HGUID parentGuid; StringToGUID(parentGuidStr.c_str(), &parentGuid);
						if (parentGuid.isValid())
						{
							auto parent = FindGameObjectByGUID(parentGuid);
							object->SetParent(parent);
						}
						//Transform
						{
							auto tranNode = item.child(L"Transform");
							glm::vec3 pos = glm::vec3(0), rot = glm::vec3(0), scale = glm::vec3(0);
							pos.x = tranNode.attribute(L"PosX").as_double();
							pos.y = tranNode.attribute(L"PosY").as_double();
							pos.z = tranNode.attribute(L"PosZ").as_double();
							object->_transform->SetLocation(pos);
							pos.x = tranNode.attribute(L"RotX").as_double();
							pos.y = tranNode.attribute(L"RotY").as_double();
							pos.z = tranNode.attribute(L"RotZ").as_double();
							object->_transform->SetRotation(rot);
							pos.x = tranNode.attribute(L"ScaX").as_double();
							pos.y = tranNode.attribute(L"ScaY").as_double();
							pos.z = tranNode.attribute(L"ScaZ").as_double();
							object->_transform->SetScale3D(scale);
						}
						//Component
						{
							auto Component = item.child(L"Component");
							for (pugi::xml_node compItem = Component.first_child(); compItem; compItem = compItem.next_sibling())
							{
								HString className = compItem.attribute(L"Class").as_string();
								auto component = object->AddComponent(className);
								auto pro = compItem.child(L"Item");

								HString proName = pro.attribute(L"Name").as_string();
								HString proType = pro.attribute(L"Type").as_string();
								HString proValue = pro.attribute(L"Value").as_string();
								

							}
						}
					}
					objectIndex++;
				}
			}
			bLoad = true;
		}
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

	auto root = _levelDoc.child(L"root");
	if (!root || root.empty())
	{
		root = _levelDoc.append_child(L"root");
	}
	for (auto g : _gameObjects)
	{
		auto Item = root.append_child(L"Item");
		Item.append_attribute(L"Type").set_value(L"GameObject");
		Item.append_attribute(L"GUID").set_value(HString(g->_guid.str().c_str()).c_wstr());
		Item.append_attribute(L"Name").set_value(g->GetObjectName().c_wstr());
		//Parent guid
		if (g->_parent)
			Item.append_attribute(L"Parent").set_value(HString(g->_parent->_guid.str().c_str()).c_wstr());
		else
			Item.append_attribute(L"Parent").set_value(HString(HGUID().str().c_str()).c_wstr());
		//Transform
		{
			auto Transform = Item.append_child(L"Transform");
			Transform.append_attribute(L"PosX").set_value(g->_transform->GetLocation().x);
			Transform.append_attribute(L"PosY").set_value(g->_transform->GetLocation().y);
			Transform.append_attribute(L"PosZ").set_value(g->_transform->GetLocation().z);
			Transform.append_attribute(L"RotX").set_value(g->_transform->GetRotation().x);
			Transform.append_attribute(L"RotY").set_value(g->_transform->GetRotation().y);
			Transform.append_attribute(L"RotZ").set_value(g->_transform->GetRotation().z);
			Transform.append_attribute(L"ScaX").set_value(g->_transform->GetScale3D().x);
			Transform.append_attribute(L"ScaY").set_value(g->_transform->GetScale3D().y);
			Transform.append_attribute(L"ScaZ").set_value(g->_transform->GetScale3D().z);
		}
		//Components
		{
			auto Component = Item.append_child(L"Component");
			for (auto c : g->_comps)
			{
				auto compItem = Component.append_child(L"Item");
				compItem.append_attribute(L"Class").set_value(c->GetComponentName().c_wstr());
				//Variables
				for (auto p : c->_compProperties)
				{
					auto valueStr = Component::AnalysisPropertyValue(p);
					if (valueStr.Length() > 0)
					{
						auto pro = compItem.append_child(L"Item");
						pro.append_attribute(L"Name").set_value(p.name.c_wstr());
						pro.append_attribute(L"Type").set_value(p.type.c_wstr());
						pro.append_attribute(L"Value").set_value(valueStr.c_wstr());
					}
				}
			}
		}
	}

	HString worldPath = _world->_worldAssetPath;
	HString levelPath = worldPath + "/" + _levelName + ".level";
	_levelDoc.save_file(levelPath.c_wstr());
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
			_world->UpdateObject(_gameObjects[i]);
		}
	}
}

void Level::AddNewObject(std::shared_ptr<GameObject> newObject)
{
	_gameObjects.push_back(newObject);
	_world->AddNewObject(newObject);
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
		_world->RemoveObject(*it);
		_gameObjects.erase(it);
	}
}

