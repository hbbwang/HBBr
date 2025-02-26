﻿#pragma once
#include "Common.h"
#include <vector>
#include <map>
#include "Transform.h"
#include "HGuid.h"
#include "Serializable.h"

class GameObject
{
	friend class World;
	friend class Level;
	friend class VulkanRenderer;
	friend class Component;
	friend class Inspector;
	friend class EditorMain;

	GameObject(std::string objectName = "NewGameObject", class Level* level = nullptr, bool SceneEditorHide = false);
	GameObject(class Level* level = nullptr, bool SceneEditorHide = false);
	GameObject(std::string objectName, std::string guidStr, class Level* level = nullptr);
public:
	~GameObject();

	void ObjectInit(std::string objectName = "NewGameObject", class Level* level = nullptr, bool SceneEditorHide = false);

	//普通的创建一个新的Object
	HBBR_API static GameObject* CreateGameObject(std::string objectName = "NewGameObject", class Level* level = nullptr);

	//创建新的Object并且自定义一个固定的GUID
	HBBR_API static GameObject* CreateGameObjectWithGUID(std::string objectName, std::string guidStr, class Level* level = nullptr);

	HBBR_API HBBR_INLINE void Destroy() {
		SetActive(false);
		_bWantDestroy = true;
	}

	HBBR_API HBBR_INLINE bool IsActive() const {
		return _bActive;
	}

	HBBR_API void SetActive(bool newActive = true);

	HBBR_API HBBR_INLINE std::string GetObjectName() const {
		return _name;
	}

	HBBR_API void SetObjectName(std::string newName);

	HBBR_API HBBR_INLINE std::vector<GameObject*> GetChildren() {
		return _children;
	}

	HBBR_API HBBR_INLINE uint64_t GetChildrenNum() {
		return _children.size();
	}

	HBBR_API HBBR_INLINE GameObject* GetParent() {
		return _parent;
	}

	HBBR_API HBBR_INLINE class Transform* GetTransform() {
		return _transform;
	}

	HBBR_API HBBR_INLINE class Level* GetLevel() {
		return _level;
	}

	HBBR_API HBBR_INLINE class World* GetWorld() {
		return _world;
	}

	HBBR_API HBBR_INLINE std::weak_ptr<GameObject> GetSelfWeekPtr() {
		return _selfWeak;
	}

	HBBR_API HBBR_INLINE HGUID GetGUID() {
		return _guid;
	}

	HBBR_API void SetParent(GameObject* newParent);

	HBBR_API void ChangeLevel(std::string newLevel);

	HBBR_API static bool IsValid(std::weak_ptr<GameObject> obj){
		return !obj.expired() && !obj.lock()->_bWantDestroy && obj.lock()->_transform != nullptr;
	}

	static std::map<std::string, std::function<class Component* (class GameObject*)>>& GetCompSpawnMap();

	template<typename T, typename ...Args>
	T* AddComponent(Args... args)
	{
		T* result = new T(this, args...);
		if (_comps.capacity() <= _comps.size())
		{
			_comps.reserve(_comps.capacity() + 5);
		}
		_comps.push_back(result);
		return result;
	}

	class Component* AddComponentByClassName(std::string className);

#if IS_EDITOR
	void* _editorObject = nullptr;
	bool _bEditorNeedUpdate = false;
	bool _sceneEditorHide = false;
	bool _bSelected = false;
	bool _bSceneOutlineVisible = true;
	bool _bSceneOutlineSearchResult = false;
#endif

	bool _IsEditorObject = false;

	bool _bEditorOpen = false;

	//嵌套层级
	int _attachmentDepth;

private:

	void Init();

	/* If object need to destroy(_bWantDesctroy = true),the function will return false. */
	bool Update();

	/* Auto run destroy execute.Do not call this function initiatively. */
	bool ExecuteDestroy();

	class World* _world = nullptr;

	class Level* _level = nullptr;

	bool _bActive = true;

	bool _bOldActive = true;

	bool _bWantDestroy = false;

	bool _bInit = false;

	std::vector<class Component*> _comps;

	std::string _name = "None";

	GameObject* _parent = nullptr;

	std::vector<GameObject*> _children;

	Transform* _transform;

	std::weak_ptr<GameObject>_selfWeak;

	HGUID _guid;

	std::string _guidStr;

	//记录场景文件里对应guid的属性,默认为空节点
	nlohmann::json _levelNode;

};
