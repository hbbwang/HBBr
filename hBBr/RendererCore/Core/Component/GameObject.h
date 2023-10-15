#pragma once
#include "Common.h"
#include <vector>
#include "HString.h"
#include "Transform.h"

class GameObject
{
	friend class SceneManager;
	friend class Component;
public:

	GameObject(HString objectName = "NewGameObject", class SceneManager* scene = NULL , bool SceneEditorHide = false);
	~GameObject();

	HBBR_API static GameObject* CreateGameObject(HString objectName = "NewGameObject", class SceneManager* scene = NULL);

	HBBR_API static GameObject* CreateModelGameObject(HString modelPath, class SceneManager* scene = NULL);

	HBBR_API HBBR_INLINE void Destroy() {
		SetActive(false);
		_bWantDestroy = true;
	}

	HBBR_API HBBR_INLINE bool IsActive() const {
		return _bActive;
	}

	HBBR_API void SetActive(bool newActive = true);

	HBBR_API HBBR_INLINE HString GetObjectName() const {
		return _name;
	}

	HBBR_API void SetObjectName(HString newName);

	HBBR_API HBBR_INLINE std::vector<GameObject*> GetChildren() {
		return _children;
	}

	HBBR_API HBBR_INLINE uint64_t GetChildrenNum() {
		return _children.size();
	}

	HBBR_API HBBR_INLINE GameObject* GetParent() {
		return _parent;
	}

	HBBR_API HBBR_INLINE Transform* GetTransform() {
		return _transform;
	}

	HBBR_API HBBR_INLINE class SceneManager* GetScene() {
		return _scene;
	}

	HBBR_API void SetParent(GameObject* newParent);

	template<typename T, typename ...Args>
	T* AddComponent(Args... args)
	{
		T* result = new T(this, args...);
		_comps.push_back(result);
		return result;
	}

	std::weak_ptr<GameObject>_selfWeak;

#if IS_EDITOR
	void* _editorObject = NULL;
	bool _bEditorNeedUpdate = false;
	bool _sceneEditorHide = false;
#endif

private:

	void Init();

	/* If object need to destroy(_bWantDesctroy = true),the function will return false. */
	bool Update();

	/* Auto run destroy execute.Do not call this function initiatively. */
	bool ExecuteDestroy();

	class SceneManager* _scene = NULL;

	bool _bActive = true;

	bool _bWantDestroy = false;

	bool _bInit = false;

	std::vector<class Component*> _comps;

	HString _name = "None";

	GameObject* _parent = NULL;

	std::vector<GameObject*> _children;

	Transform* _transform;

};
