#pragma once

//世界类,每个渲染器必须有一个场景管理类,用来储存当前场景的所有对象(GameObject)
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
class World : public AssetObject
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
	friend class Level;
public:

	HBBR_API static std::weak_ptr<World> LoadAsset(HGUID guid);

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

	//添加关卡,
	//1.关卡资产名称
	//2.关卡资产的Asset虚拟路径
	//3.关卡资产的Asset绝对路径
	HBBR_API void AddLevel(HString levelNameOrContentPath);

	HBBR_API void AddLevel(HGUID guid);

	HBBR_API void AddEmptyLevel(HString newLevelName = "NewLevel");

private:

	//加载场景
	void Load(class VulkanRenderer* renderer);

	//释放场景,但是asset依然存在
	bool UnLoad();

	//释放场景,包括.world asset.
	bool ReleaseWorld();

	void WorldUpdate();

	void AddNewObject(std::shared_ptr<GameObject> newObject);

	void RemoveObject(GameObject* object);

	class VulkanRenderer* _renderer = NULL;

	class CameraComponent* _mainCamera = NULL;

	std::vector<CameraComponent*> _cameras;

	std::vector<std::shared_ptr<GameObject>> _gameObjects;

	//需要进行销毁的Objects
	std::vector<std::shared_ptr<GameObject>> _gameObjectNeedDestroy;

	//Test 
	//std::weak_ptr<GameObject> testObj;

	std::vector<std::weak_ptr<Level>> _levels;

	//Reference Count 可能存在多个VulkanRenderer共用一个World的情况，需要记录下来
	int _refCount = 0;
};



