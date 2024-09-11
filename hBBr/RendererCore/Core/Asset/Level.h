#pragma once
#include "Asset/AssetObject.h"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Component/GameObject.h"
#include "Asset/HGuid.h"
#include "Asset/Serializable.h"

class Level :public Serializable
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
	friend class World;
	friend class EditorMain;
	
public:

	Level(class World* world, HString name);
	~Level();

	HBBR_INLINE HBBR_API HString GetLevelName()const { return _levelName; }

	HBBR_API void Rename(HString newName);

	HBBR_API GameObject* FindGameObjectByGUID(HGUID guid);

	HBBR_API HString GetLevelAbsPath()const { return _levelAbsPath; }

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

	HBBR_API HGUID GetGUID()const { return _guid; }

#if IS_EDITOR

	HBBR_API void DeleteLevel(bool bImmediately = false);

	HBBR_API void MarkDirty();

	static std::vector<std::weak_ptr<Level>> _dirtyLevels;
	HBBR_API static std::vector<std::weak_ptr<Level>> GetDirtyLevels() { return _dirtyLevels; }

	bool bDirtySelect = true;
	std::vector<std::function<void()>> dirtyFunc;

	HBBR_API  std::vector<std::function<void()>> GetDirtyFunc() { return dirtyFunc; }

	HBBR_API  bool IsDirtySelect() { return bDirtySelect; }

	HBBR_API  void SetDirtySelect(bool input) { bDirtySelect = input; }

	HBBR_API static void AddDirtyLevel(std::weak_ptr<Level> dirtyLevel)
	{
		auto it = std::find_if(_dirtyLevels.begin(), _dirtyLevels.end(), [dirtyLevel](std::weak_ptr<Level>& w) {
			return !w.expired() && !dirtyLevel.expired() && w.lock().get() == dirtyLevel.lock().get() && w.lock()->GetLevelName() == dirtyLevel.lock()->GetLevelName();
		});
		if (it == _dirtyLevels.end())
		{
			_dirtyLevels.push_back(dirtyLevel);
		}
	}

	HBBR_API static void RemoveDirtyLevel(std::weak_ptr<Level> dirtyLevel)
	{
		auto it = std::remove_if(_dirtyLevels.begin(), _dirtyLevels.end(), [dirtyLevel](std::weak_ptr<Level>& w) {
			return !w.expired() && !dirtyLevel.expired() && w.lock().get() == dirtyLevel.lock().get();
			});
		if (it != _dirtyLevels.end())
		{
			_dirtyLevels.erase(it);
		}
	}

	HBBR_API static void ClearDirtyLevels()
	{
		_dirtyLevels.clear();
	}

	HBBR_API HBBR_INLINE bool IsActive() const {
		return _bActive;
	}

	HBBR_API void SetActive(bool newActive = true);

	bool _bSelected = false;

	bool _bEditorOpen = false;

#endif

private:

	void LevelUpdate();

	bool _bActive = true;

	bool _bOldActive = true;

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
	void AddNewObject(std::shared_ptr<GameObject> newObject, bool bSkipWorldCallback = false);

	//请勿要主动使用该函数
	void RemoveObject(GameObject* object,bool bNotDestoryObject = false);

	//游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjects;

	//需要进行销毁的游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjectNeedDestroy;

	std::map<HGUID, AssetType>_dependency;

	bool _isEditorLevel = false;

	HGUID _guid;

public:
	virtual nlohmann::json ToJson()override;
	virtual void FromJson() override;

};



