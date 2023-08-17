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

	GameObject(HString objectName = "NewGameObject", class SceneManager* scene = NULL);
	~GameObject();

	HBBR_API __forceinline static GameObject* CreateGameObject(HString objectName = "NewGameObject", class SceneManager* scene = NULL)
	{
		return new GameObject(objectName, scene);
	}

	HBBR_API __forceinline void Destroy() {
		SetActive(false);
		_bWantDestroy = true;
	}

	HBBR_API __forceinline bool IsActive() const {
		return _bActive;
	}

	HBBR_API void SetActive(bool newActive = true);

	HBBR_API __forceinline HString GetObjectName() const {
		return _name;
	}

	HBBR_API void SetObjectName(HString newName);

	HBBR_API __forceinline std::vector<GameObject*> GetChildren() {
		return _children;
	}

	HBBR_API __forceinline uint64_t GetChildrenNum() {
		return _children.size();
	}

	HBBR_API __forceinline GameObject* GetParent() {
		return _parent;
	}

	HBBR_API __forceinline Transform* GetTransform() {
		return _transform;
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
#endif

private:

	void Init();

	/* If object need to destroy(_bWantDesctroy = true),the function will return false. */
	bool Update();

	/* Auto run destroy execute.Do not call this function initiatively. */
	bool ExecuteDestroy();

	class SceneManager* _scene = NULL;

	bool _bActive;

	bool _bWantDestroy;

	bool _bInit;

	std::vector<class Component*> _comps;

	HString _name;

	GameObject* _parent = NULL;

	std::vector<GameObject*> _children;

	Transform* _transform;

};
