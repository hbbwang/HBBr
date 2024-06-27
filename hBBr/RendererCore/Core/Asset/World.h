#pragma once

//世界类
//世界文件后缀为.world
//场景的结构由多个场景（.level）组成
#include "Asset/AssetObject.h"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Component/GameObject.h"
#include "Asset/HGuid.h"
#include "Asset/Level.h"
#include "Asset/Serializable.h"

class World :public Serializable
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
	friend class Level;
public:
	World();
	~World();

#if IS_EDITOR
	class CameraComponent* _editorCamera = nullptr;
#endif

	HBBR_API HBBR_INLINE class VulkanRenderer* GetRenderer()const { return _renderer; }

	HBBR_API HBBR_INLINE class CameraComponent* GetMainCamera()const { return _mainCamera; }

	HBBR_API HBBR_INLINE std::vector<std::shared_ptr<Level>> GetLevels()const { return _levels; }

	HBBR_API Level* GetLevel(HString name);

	HBBR_API void AddNewLevel(HString name);

	HBBR_API void DeleteLevel(HString levelName);

	HBBR_API const HGUID GetGUID()const { return _guid; }

	HBBR_API static std::shared_ptr<World>  CreateNewWorld(HString newWorldName);

	HBBR_API void SetWorldName(HString name);

	HBBR_API HString GetWorldName()const { return _worldName; }

	//保存世界,路径都是固定在Asset/World里
	//文件夹内储存的则是level后缀
	HBBR_API void SaveWorld(HString newWorldName = "");

	// 创建/保存 世界设置，世界的属性和参数都在这里
	HBBR_API void SaveWorldSetting();

	//重新读取世界设置
	HBBR_API void ReloadWorldSetting();

	HBBR_API GameObject* SpawnGameObject(HString name, class Level* level = nullptr);

#if IS_EDITOR

	std::weak_ptr<Level> _currentSelectionLevel;

	std::function<void(std::shared_ptr<GameObject>)> _editorGameObjectAddFunc = [](std::shared_ptr<GameObject> newObject) {};
	std::function<void(std::shared_ptr<GameObject>)> _editorGameObjectRemoveFunc = [](std::shared_ptr<GameObject> oldObject) {};
	std::function<void(std::shared_ptr<GameObject>)> _editorGameObjectUpdateFunc = [](std::shared_ptr<GameObject> oldObject) {};
	std::function<void(std::shared_ptr<GameObject>,  std::shared_ptr<GameObject>)> _editorGameObjectSetParentFunc = [](std::shared_ptr<GameObject> Object, std::shared_ptr<GameObject> newParent) {};
	std::function<void(std::vector<std::weak_ptr<Level>>&)> _editorWorldUpdate = [](std::vector<std::weak_ptr<Level>> levels) {};
	std::function<void()> _editorLevelChanged = []() {};
	std::function<void(Level*, bool)> _editorLevelVisibilityChanged = [](Level*, bool) {};

	std::shared_ptr<Level> _editorLevel;

	void SetCurrentSelectionLevel(std::weak_ptr<Level> level);

	HBBR_API void MarkDirty();

	static std::vector<std::weak_ptr<World>> _dirtyWorlds;

	HBBR_API static std::vector<std::weak_ptr<World>> GetDirtyWorlds() { return _dirtyWorlds; }

	bool bDirtySelect = true;

	HBBR_API  bool IsDirtySelect() { return bDirtySelect; }

	HBBR_API  void SetDirtySelect(bool input) { bDirtySelect = input; }

	HBBR_API static void AddDirtyWorld(std::weak_ptr<World> dirtyWorld)
	{
		auto it = std::find_if(_dirtyWorlds.begin(), _dirtyWorlds.end(), [dirtyWorld](std::weak_ptr<World>& w) {
			return !w.expired() && !dirtyWorld.expired() && w.lock().get() == dirtyWorld.lock().get() && w.lock()->GetWorldName()== dirtyWorld.lock()->GetWorldName();
		});
		if (it == _dirtyWorlds.end())
		{
			_dirtyWorlds.push_back(dirtyWorld);
		}
	}
	HBBR_API static void RemoveDirtyWorld(std::weak_ptr<World> dirtyWorld)
	{
		auto it = std::remove_if(_dirtyWorlds.begin(), _dirtyWorlds.end(), [dirtyWorld](std::weak_ptr<World>& w) {
			return !w.expired() && !dirtyWorld.expired() && w.lock().get() == dirtyWorld.lock().get();
			});
		if (it != _dirtyWorlds.end())
		{
			_dirtyWorlds.erase(it);
		}
	}
	HBBR_API static void ClearDirtyWorlds()
	{
		_dirtyWorlds.clear();
	}


#endif

private:

	//加载场景资产
	void Load(class VulkanRenderer* renderer);

	//释放场景
	bool ReleaseWorld();

	void WorldUpdate();

	//请勿要主动使用该函数
	void UpdateObject(std::shared_ptr<GameObject> newObject);

	//请勿要主动使用该函数
	void AddNewObject(std::shared_ptr<GameObject> newObject);

	//请勿要主动使用该函数
	void RemoveObject(std::shared_ptr<GameObject> object);

	class VulkanRenderer* _renderer = nullptr;

	class CameraComponent* _mainCamera = nullptr;

	std::vector<CameraComponent*> _cameras;

	std::vector<std::shared_ptr<Level>> _levels;

	HString _worldName = "NewWorld";

	HString _worldAbsPath = "";

	HString _worldSettingAbsPath = "";

	bool _bLoad = false;

	HGUID _guid;

	HString _guidStr;

	//World Setting
	public:
		nlohmann::json ToJson()  override;
		void FromJson() override;

};



