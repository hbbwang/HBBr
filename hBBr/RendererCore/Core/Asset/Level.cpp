#include "Asset/Level.h"
#include "Asset/World.h"
#include "FileSystem.h"
#include "XMLStream.h"
#include "Component/Component.h"

Level::Level(class World* world, HString name)
{
	_levelName = name.ClearSpace();
	_levelAbsPath = FileSystem::Append(world->_worldAbsPath, _levelName) + ".level";
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
	if (FileSystem::FileExist(_levelAbsPath))
	{
		FileSystem::FileRemove(_levelAbsPath.c_str());
		_levelAbsPath = FileSystem::Append(_world->_worldAbsPath, _levelName) + ".level";
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
		if (!_bLoad)
		{
			_bLoad = true;
			bool bLoadFileSucceed = false;
			if(!_isEditorLevel)
				bLoadFileSucceed = LoadJson(_levelAbsPath);
			//
			if (!bLoadFileSucceed)
			{
				_json.clear();
			}
			else
			{
				struct PickObject {
					GameObject* obj = nullptr;
					HString type;
					HString name;
					nlohmann::json item;
				};
				std::vector<PickObject> objs;
				//先 生成
				{
					_json = _json;
					for (auto& item : _json.items())
					{
						HGUID guid(item.key());
						auto i = item.value();
						HString name = i["Name"];
						GameObject* newObject = new GameObject(name, guid.str(), this);
						newObject->_levelNode = i;
						if (objs.capacity() < objs.size())
							objs.reserve(objs.capacity() + 25);
						PickObject newPick;
						newPick.obj = newObject;
						newPick.item = i;
						newPick.name = name;
						objs.push_back(newPick);
					}
				}
				//再考虑父子关系
				for (auto & i : objs)
				{
					auto object = i.obj;
					//Parent
					HGUID parentGuid; to_json(i.item["Parent"], parentGuid);
					if (parentGuid.isValid())
					{
						auto parent = FindGameObjectByGUID(parentGuid);
						object->SetParent(parent);
					}
					//Transform
					{
						nlohmann::json tran = i.item["Transform"];
						glm::vec3 pos;		from_json(tran["Position"], pos);
						glm::vec3 rot;		from_json(tran["Rotation"], rot);
						glm::vec3 sca;		from_json(tran["Scale"], sca);
						object->_transform->SetLocation(pos);
						object->_transform->SetRotation(rot);
						object->_transform->SetScale3D(sca);
					}
					//Component
					{
						std::vector<nlohmann::json>comps = i.item["Components"];
						for (auto& c : comps)
						{
							HString className = c["Class"];
							auto component = object->AddComponent(className);
							std::vector<nlohmann::json>compPros = c["Properties"];
							for (auto& p : compPros)
							{
								HString proName = p["Name"];
								HString proType =  p["Type"];
								HString proValue = p["Value"];
								for (auto& pp : component->_compProperties)
								{
									if (pp.name == proName && pp.type == proType)
									{
										Component::StringToPropertyValue(pp, proValue);
										break;
									}
								}
							}
							component->UpdateData();
						}
					}
				}
				//
			}
		}
	}

#if IS_EDITOR
	_world->_editorLevelVisibilityChanged(this,_bLoad);
#endif
}

void Level::UnLoad()
{
	_bLoad = false;

	//临时保存一份数据在内存中，不会实际保存到文件
	for (auto g : _gameObjects)
	{
		SaveGameObject(g.get());
	}

	//
	for (auto& i : this->_gameObjects)
	{
		i->Destroy();
	}

#if IS_EDITOR
	_world->_editorLevelVisibilityChanged(this, _bLoad);
#endif
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
		SaveGameObject(g.get());
	}
	SaveJson(_levelAbsPath);
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

nlohmann::json Level::ToJson()
{
	return _json;
}

void Level::FromJson()
{

}

//---------------------------------------------------------------------------------XML Document-------------
#pragma region XML Document
void Level::SaveGameObject(GameObject* g)
{
	//我们不在乎编辑器相关的内容
	if (_isEditorLevel || g->_IsEditorObject)
		return;

	g->_levelNode["Name"] = g->_name;
	if (g->_parent)
		g->_levelNode["Parent"] = g->_parent->_guid;
	else
		g->_levelNode["Parent"] = HGUID();

	//Transform and parent
	SaveGameObjectTransform(g);
	//Components
	SaveGameObjectComponent(g);
	//Save
	_json[g->_guid.str()] = g->_levelNode;
}

void Level::SaveGameObjectTransform(GameObject* g)
{
	if (g == nullptr)
		return;

	nlohmann::json tran;
	nlohmann::json pos;	to_json(pos, g->_transform->GetLocation());
	nlohmann::json rot;		to_json(rot, g->_transform->GetRotation());
	nlohmann::json sca;		to_json(sca, g->_transform->GetScale3D());
	tran["Position"] = pos;		tran["Rotation"] = rot;		tran["Scale"] = sca;
	g->_levelNode["Transform"] = tran;

}

void Level::SaveGameObjectComponent(GameObject* g)
{
	if (g == nullptr)
		return;

	std::vector<nlohmann::json> comps;
	comps.reserve(g->_comps.size());
	int compIndex = 0;
	for (auto c : g->_comps)
	{
		nlohmann::json sub;
		std::vector<nlohmann::json> subPros;
		subPros.reserve(c->_compProperties.size());
		sub["Index"] = compIndex;
		sub["Class"] = c->GetComponentName();
		//Variables
		for (auto p : c->_compProperties)
		{
			nlohmann::json subPro;
			auto valueStr = Component::PropertyValueToString(p);
			if (valueStr.Length() > 0)
			{
				subPro["Name"] = p.name;
				subPro["Type"] = p.type;
				subPro["Value"] = valueStr;
				subPros.push_back(subPro);
			}
		}
		sub["Properties"] = subPros;
		comps.push_back(sub);
		compIndex++;
	}
	g->_levelNode["Components"] = comps;
}

#pragma endregion XML Document