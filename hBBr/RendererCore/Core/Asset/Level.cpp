#include "Asset/Level.h"
#include "Asset/World.h"
#include "FileSystem.h"
#include "XMLStream.h"
#include "Component/Component.h"

Level::Level(class World* world, HString name)
{
	_levelDoc = pugi::xml_document();
	_levelName = name.ClearSpace();
	_levelPath = FileSystem::Append(world->_worldAssetPath, _levelName) + ".level";
	_world = world;
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
	_levelName = newName.ClearSpace();
	if (FileSystem::FileExist(_levelPath))
	{
		FileSystem::FileRemove(_levelPath.c_str());
		_levelPath = FileSystem::Append(_world->_worldAssetPath, _levelName) + ".level";
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

void Level::Load()
{
	if (_world)
	{
		if (!bLoad)
		{
			bLoad = true;
			int count = 0;
			for (auto i : _levelDoc.children())
				count++;
			if (count ==0)
			{
				if (XMLStream::LoadXML(_levelPath.c_wstr(), _levelDoc))
				{
					_levelRoot = _levelDoc.child(TEXT("root"));
					if (!_levelRoot)
						_levelRoot = _levelDoc.append_child(L"root");
				}
				else
				{
					//这里我们不打算保存该xml,保存的决定留给用户
					XMLStream::CreateXMLDocument("?", _levelDoc);
					_levelRoot = _levelDoc.append_child(TEXT("root"));
				}
			}
			//
			{
				struct PickObject {
					GameObject* obj = nullptr;
					HString type;
					HString name;
					pugi::xml_node item;
				};
				std::vector<PickObject> objs;
				//先 生成
				{
					for (pugi::xml_node item = _levelRoot.first_child(); item; item = item.next_sibling())
					{
						HString type = item.attribute(L"Type").as_string();
						if (type == "GameObject")
						{
							HString name = item.attribute(L"Name").as_string();
							HString guidStr = item.attribute(L"GUID").as_string();
							GameObject* newObject = new GameObject(name, guidStr, this);
							//另存 xmlNode，这样就不用每次都去levelRoot里大范围遍历搜索了。
							newObject->_xmlNode = item;
							//
							if (objs.capacity() < objs.size())  objs.reserve(objs.capacity() + 25);
							PickObject newPick;
							newPick.obj = newObject;
							newPick.item = item;
							newPick.type = type;
							newPick.name = name;
							objs.push_back(newPick);
						}
					}
				}
				//再考虑父子关系
				for (auto & i : objs)
				{
					if (i.type == "GameObject")
					{
						auto object = i.obj;
						//Parent
						HString parentGuidStr = i.item.attribute(L"Parent").as_string();
						HGUID parentGuid; StringToGUID(parentGuidStr.c_str(), &parentGuid);
						if (parentGuid.isValid())
						{
							auto parent = FindGameObjectByGUID(parentGuid);
							object->SetParent(parent);
						}
						//Transform
						{
							auto tranNode = i.item.child(L"Transform");
							glm::vec3 pos = glm::vec3(0), rot = glm::vec3(0), scale = glm::vec3(0);
							pos.x = (float)tranNode.attribute(L"PosX").as_double();
							pos.y = (float)tranNode.attribute(L"PosY").as_double();
							pos.z = (float)tranNode.attribute(L"PosZ").as_double();
							object->_transform->SetLocation(pos);
							pos.x = (float)tranNode.attribute(L"RotX").as_double();
							pos.y = (float)tranNode.attribute(L"RotY").as_double();
							pos.z = (float)tranNode.attribute(L"RotZ").as_double();
							object->_transform->SetRotation(rot);
							pos.x = (float)tranNode.attribute(L"ScaX").as_double();
							pos.y = (float)tranNode.attribute(L"ScaY").as_double();
							pos.z = (float)tranNode.attribute(L"ScaZ").as_double();
							object->_transform->SetScale3D(scale);
						}
						//Component
						{
							auto Component = i.item.child(L"Component");
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
				}
				//
			}
		}
	}
}

void Level::UnLoad()
{
	bLoad = false;

	//临时保存一份数据在内存中，不会实际保存到XML里
	for (auto g : _gameObjects)
	{
		XML_UpdateGameObject(g.get());
	}

	//
	for (auto& i : this->_gameObjects)
	{
		i->Destroy();
	}
}

void Level::SaveLevel()
{
	//我们不在乎编辑器相关的内容
	if (_isEditorLevel)
	{
		return;
	}

	if (!_world)
		return;

	for (auto g : _gameObjects)
	{
		XML_UpdateGameObject(g.get());
	}
	_levelDoc.save_file(_levelPath.c_wstr());
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

//---------------------------------------------------------------------------------XML Document-------------
#pragma region XML Document
void Level::XML_UpdateGameObject(GameObject* g)
{
	//我们不在乎编辑器相关的内容
	if (_isEditorLevel || g->_IsEditorObject)
		return;
	//如果节点为空，则在LevelRoot里根据GUID查找，找不到就建一个新的
	if (g->_xmlNode.empty())
	{
		g->_xmlNode = _levelRoot.find_child_by_attribute(TEXT("GUID"), g->_guidStr.c_wstr());
		if (g->_xmlNode.empty())
		{
			g->_xmlNode = _levelRoot.append_child(L"Item");
			g->_xmlNode.append_attribute(L"Type").set_value(L"GameObject");
			g->_xmlNode.append_attribute(L"GUID").set_value(g->_guidStr.c_wstr());
		}
	}
	//Transform and parent
	XML_UpdateGameObjectTransform(g);
	//Components
	XML_UpdateGameObjectComponent(g);
}

void Level::XML_UpdateGameObjectTransform(GameObject* g)
{
	g->_xmlNode.append_attribute(L"Name").set_value(g->GetObjectName().c_wstr());
	//Parent guid
	if (g->_parent)
		g->_xmlNode.append_attribute(L"Parent").set_value(g->_parent->_guidStr.c_wstr());
	else
		g->_xmlNode.append_attribute(L"Parent").set_value(HString(HGUID().str().c_str()).c_wstr());
	//Transform
	{
		auto Transform = g->_xmlNode.append_child(L"Transform");
		auto pos = g->_transform->GetLocation();
		auto rota = g->_transform->GetRotation();
		auto scale = g->_transform->GetScale3D();
		Transform.append_attribute(L"PosX").set_value(pos.x);
		Transform.append_attribute(L"PosY").set_value(pos.y);
		Transform.append_attribute(L"PosZ").set_value(pos.z);
		Transform.append_attribute(L"RotX").set_value(rota.x);
		Transform.append_attribute(L"RotY").set_value(rota.y);
		Transform.append_attribute(L"RotZ").set_value(rota.z);
		Transform.append_attribute(L"ScaX").set_value(scale.x);
		Transform.append_attribute(L"ScaY").set_value(scale.y);
		Transform.append_attribute(L"ScaZ").set_value(scale.z);
	}
}

void Level::XML_UpdateGameObjectComponent(GameObject* g)
{
	auto Component = g->_xmlNode.append_child(L"Component");
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


#pragma endregion XML Document