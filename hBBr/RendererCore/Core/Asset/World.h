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
#include "./ThirdParty/pugixml/pugixml.hpp"
#include "Asset/HGuid.h"
#include "Asset/Level.h"
class World
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
	friend class Level;
public:
	World(class VulkanRenderer* renderer);
	~World();

#if IS_EDITOR
	std::function<void(class World*, std::vector<std::shared_ptr<GameObject>>)> _editorSceneUpdateFunc = [](class World* world, std::vector<std::shared_ptr<GameObject>> o) {};
	std::function<void(class World*, std::shared_ptr<GameObject>)> _editorGameObjectAddFunc = [](class World* world, std::shared_ptr<GameObject> newObject) {};
	std::function<void(class World*, std::shared_ptr<GameObject>)> _editorGameObjectRemoveFunc = [](class World* world, std::shared_ptr<GameObject> oldObject) {};
	std::function<void(class World*, std::shared_ptr<GameObject>)> _editorGameObjectUpdateFunc = [](class World* world, std::shared_ptr<GameObject> oldObject) {};
	class CameraComponent* _editorCamera = NULL;
#endif

	HBBR_API HBBR_INLINE class VulkanRenderer* GetRenderer()const { return _renderer; }

	HBBR_API HBBR_INLINE class CameraComponent* GetMainCamera()const { return _mainCamera; }

	HBBR_API void AddNewLevel(HString name);

	//保存世界xml,路径都是固定在Asset/World里
	//World 的结构大致如下:
	//'Asset/World/$(_worldName).world/' 这个$(_worldName).world是一个文件夹。
	//文件夹内储存的则是level后缀的xml文件
	HBBR_API void SaveWorld();

	//保存世界xml,包括Levels
	HBBR_API void SaveWholeWorld();

#if IS_EDITOR

	std::weak_ptr<Level> _currentSelectionLevel;

	static std::map<HGUID, std::function<void(VulkanRenderer*, World*)>> _editorSpwanNewWorld;

	std::map<HGUID, std::function<void(class World*, std::vector<Level*>)>> _editorWorldUpdate;


	void SetCurrentSelectionLevel(std::weak_ptr<Level> level);

	HBBR_API HBBR_INLINE static void AddSpawnNewWorldCallBack_Editor(HGUID guid, std::function<void(VulkanRenderer*, World*)> func)
	{
		_editorSpwanNewWorld.emplace(guid, func);
	}

	HBBR_API HBBR_INLINE static void RemoveSpawnNewWorldCallBack_Editor(HGUID guid)
	{
		_editorSpwanNewWorld.erase(guid);
	}

#endif

private:

	//加载场景
	void Load(class VulkanRenderer* renderer);

	//释放场景
	bool ReleaseWorld();

	void WorldUpdate();

	void AddNewObject(std::shared_ptr<GameObject> newObject);

	void RemoveObject(GameObject* object);

	class VulkanRenderer* _renderer = NULL;

	class CameraComponent* _mainCamera = NULL;

	std::vector<CameraComponent*> _cameras;

	//游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjects;

	//需要进行销毁的游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjectNeedDestroy;

	std::vector<std::shared_ptr<Level>> _levels;

	HString _worldName = "NewWorld";

	bool bLoad = false;
};



