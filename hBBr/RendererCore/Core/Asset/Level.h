#pragma once
#include "Asset/AssetObject.h"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Component/GameObject.h"
#include "./ThirdParty/pugixml/pugixml.hpp"
#include "Asset/HGuid.h"

class Level 
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
	friend class World;
public:

	Level(class World* world, HString name);
	~Level();

	//序列化
	template<class Archive>
	void serialize(Archive& ar) {
		//ar(name_, age_);
	}

	HBBR_INLINE HBBR_API HString GetLevelName()const { return _levelName; }

	HBBR_API void Rename(HString newName);

	HBBR_API GameObject* FindGameObjectByGUID(HGUID guid);

	//Load level
	HBBR_API void Load();

	//Release level
	HBBR_API void UnLoad();

	HBBR_API void SaveLevel();

	HBBR_API const bool IsLoaded()const { return bLoad; }

	//Get all game objects from this level.
	HBBR_API std::vector<std::shared_ptr<GameObject>>& GetAllGameObjects() {
		return _gameObjects;
	}

	HBBR_API class World* GetWorld()const { return _world; }

private:

	void LevelUpdate();

	bool bLoad = false;

	class World* _world = nullptr;

	HString _levelName = "NewLevel";

	HString _levelPath = "";

	pugi::xml_document _levelDoc;
	pugi::xml_node _levelRoot;

	//更新或者创建GameObject的Xml数据在level文档里
	//bUpdateParameters是否写入参数
	void XML_UpdateGameObject(GameObject* gameObject);
	void XML_UpdateGameObjectTransform(GameObject* gameObject);
	void XML_UpdateGameObjectComponent(GameObject* gameObject);

	//请勿要主动使用该函数
	void AddNewObject(std::shared_ptr<GameObject> newObject);

	//请勿要主动使用该函数
	void RemoveObject(GameObject* object);

	//游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjects;

	//需要进行销毁的游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjectNeedDestroy;

	bool _isEditorLevel = false;
};



