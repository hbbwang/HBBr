#pragma once

#include "HString.h"

class GameObject
{
	friend class SceneManager;
public:

	GameObject(class SceneManager* scene = NULL);
	~GameObject();

	__forceinline void Destroy() {
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

private:

	void Init();

	/* If object need to destroy(_bWantDesctroy = true),the function will return false. */
	bool Update();

	class SceneManager* _scene = NULL;

	bool _bActive;

	bool _bWantDestroy = false;

	HString _name = "NewGameObject";
};
