#pragma once

#include "HString.h"

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

	template<typename T, typename ...Args>
	T* AddComponent(Args... args)
	{
		T* newComp = new T(this, args...);
		_comps.push_back(newComp);
		return newComp;
	}

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

	std::vector<class Component*> _comps;

	HString _name;
};
