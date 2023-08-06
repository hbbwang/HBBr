#pragma once
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

	__forceinline void Destroy() {
		SetActive(false);
		_bWantDestroy = true;
	}

	__forceinline bool IsActive() const {
		return _bActive;
	}

	void SetActive(bool newActive = true);

	__forceinline HString GetObjectName() const {
		return _name;
	}

	__forceinline void SetObjectName(HString newName) {
		_name = newName;
	}

	__forceinline std::vector<GameObject*> GetChildren() {
		return _children;
	}

	__forceinline GameObject* GetParent() {
		return _parent;
	}

	__forceinline Transform* GetTransform() {
		return _transform;
	}

	void SetParent(GameObject* newParent);

	template<typename T, typename ...Args>
	T* AddComponent(Args... args)
	{
		std::unique_ptr<T> newComp;
		newComp.reset(new T(this, args...));
		T* result = newComp.get();
		_comps.push_back(std::move(newComp));
		return result;
	}

	std::weak_ptr<GameObject>_selfWeak;

private:

	void Init();

	/* If object need to destroy(_bWantDesctroy = true),the function will return false. */
	bool Update();

	/* Auto run destroy execute.Do not call this function initiatively. */
	void ExecuteDestroy();

	class SceneManager* _scene = NULL;

	bool _bActive;

	bool _bWantDestroy;

	bool _bInit;

	std::vector<std::unique_ptr<class Component>> _comps;

	HString _name;

	//指定新的父子关系,但并不是立刻执行的，将会在SceneManager的Update里统一进行处理
	GameObject* _newParent = NULL;

	GameObject* _parent = NULL;

	std::vector<GameObject*> _children;

	Transform* _transform;

};
