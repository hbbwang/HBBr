#pragma once
#include "Asset/AssetObject.h"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Component/GameObject.h"
#include "./ThirdParty/pugixml/pugixml.hpp"
#include "Asset/HGuid.h"
#include "Asset/Serializable.h"

class Level :public Serializable
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
	friend class World;
public:

	Level(class World* world, HString name);
	~Level();

	HBBR_INLINE HBBR_API HString GetLevelName()const { return _levelName; }

	HBBR_API void Rename(HString newName);

	HBBR_API GameObject* FindGameObjectByGUID(HGUID guid);

	//Load level
	HBBR_API void Load();

	//Release level
	HBBR_API void UnLoad();

	HBBR_API void SaveLevel();

	HBBR_API const bool IsLoaded()const { return _bLoad; }

	//Get all game objects from this level.
	HBBR_API std::vector<std::shared_ptr<GameObject>>& GetAllGameObjects() {
		return _gameObjects;
	}

	HBBR_API class World* GetWorld()const { return _world; }

private:

	void LevelUpdate();

	bool _bLoad = false;

	class World* _world = nullptr;

	HString _levelName = "NewLevel";

	HString _levelAbsPath = "";

	//更新或者创建GameObject的Xml数据在level文档里
	//bUpdateParameters是否写入参数
	void SaveGameObject(GameObject* gameObject);
	void SaveGameObjectTransform(GameObject* gameObject);
	void SaveGameObjectComponents(GameObject* gameObject);

	//请勿要主动使用该函数
	void AddNewObject(std::shared_ptr<GameObject> newObject);

	//请勿要主动使用该函数
	void RemoveObject(GameObject* object);

	//游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjects;

	//需要进行销毁的游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjectNeedDestroy;

	bool _isEditorLevel = false;
	bool _bInitVisibility = false;

public:
	virtual nlohmann::json ToJson()override;
	virtual void FromJson() override;

};



