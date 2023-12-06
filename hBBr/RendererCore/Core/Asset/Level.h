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

	Level() {}
	Level(HString name);
	~Level();

	HBBR_INLINE HBBR_API HString GetLevelName()const { return _levelName; }

	//加载关卡
	HBBR_API void Load(class World* world);

	//释放关卡对象
	HBBR_API bool UnLoad();

	//释放关卡,包括对象和Xml Doc
	HBBR_API bool ResetLevel();

	HBBR_API void SaveLevel();

	HBBR_API class World* GetWorld()const { return _world; }

#if IS_EDITOR
	std::function<void(class Level*, std::shared_ptr<GameObject>)> _editorGameObjectAddFunc = [](class Level* world, std::shared_ptr<GameObject> newObject) {};
	std::function<void(class Level*, std::shared_ptr<GameObject>)> _editorGameObjectRemoveFunc = [](class Level* world, std::shared_ptr<GameObject> oldObject) {};
	std::function<void(class Level*, std::shared_ptr<GameObject>)> _editorGameObjectUpdateFunc = [](class Level* world, std::shared_ptr<GameObject> oldObject) {};
	std::function<void(class Level*, std::vector<std::shared_ptr<GameObject>>)> _editorSceneUpdateFunc = [](class Level* world, std::vector<std::shared_ptr<GameObject>> o) {};
#endif

private:

	void LevelUpdate();

	bool bLoad = false;

	class World* _world = NULL;

	HString _levelName = "NewLevel";

	pugi::xml_document _levelDoc;

	void AddNewObject(std::shared_ptr<GameObject> newObject);

	void RemoveObject(GameObject* object);

	//游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjects;

	//需要进行销毁的游戏对象
	std::vector<std::shared_ptr<GameObject>> _gameObjectNeedDestroy;

	bool _isEditorLevel = false;
};



