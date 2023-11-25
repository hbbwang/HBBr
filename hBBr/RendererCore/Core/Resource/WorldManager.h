#pragma once

//世界类,每个渲染器必须有一个场景管理类,用来储存当前场景的所有对象(GameObject)
//世界文件后缀为.world
//场景的结构由多个场景（.scene）组成

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Component/GameObject.h"
#include "./ThirdParty/pugixml/pugixml.hpp"
#include "Resource/HGuid.h"

typedef void (*EditorSceneUpdate)(class WorldManager* scene, std::vector<std::shared_ptr<GameObject>> aliveObjects);
typedef void (*EditorGameObjectAdd)(class WorldManager* scene, std::shared_ptr<GameObject> newObject);
typedef void (*EditorGameObjectRemove)(class WorldManager* scene, std::shared_ptr<GameObject> oldObject);

class WorldManager
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
public:

	~WorldManager();

#if IS_EDITOR
	std::function<void(class WorldManager*, std::vector<std::shared_ptr<GameObject>>)> _editorSceneUpdateFunc = [](class WorldManager* s, std::vector<std::shared_ptr<GameObject>> o) {};
	std::function<void(class WorldManager*, std::shared_ptr<GameObject>)> _editorGameObjectAddFunc = [](class WorldManager* scene, std::shared_ptr<GameObject> newObject) {};
	std::function<void(class WorldManager*, std::shared_ptr<GameObject>)> _editorGameObjectRemoveFunc = [](class WorldManager* scene, std::shared_ptr<GameObject> oldObject) {};
	std::function<void(class WorldManager*, std::shared_ptr<GameObject>)> _editorGameObjectUpdateFunc = [](class WorldManager* scene, std::shared_ptr<GameObject> oldObject) {};
	class CameraComponent* _editorCamera = NULL;
#endif

	HBBR_API HBBR_INLINE class VulkanRenderer* GetRenderer()const { return _renderer; }

	HBBR_API HBBR_INLINE class CameraComponent* GetMainCamera()const { return _mainCamera; }

private:

	void WorldInit(class VulkanRenderer* renderer);

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
	std::weak_ptr<GameObject> testObj;
};



