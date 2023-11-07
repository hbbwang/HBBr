#pragma once
//基础Compoennt组件类,组件都是附加在GameObject上的,结构体参数必须至少带有class GameObject* parent
#include "Common.h"
#include <vector>
class Component
{
	friend class GameObject;
	friend class SceneManager;
	friend class VulkanRenderer;
public:

	Component(class GameObject* parent);

	virtual  ~Component();

	void Destroy();

	HBBR_INLINE bool IsActive() const {
		return _bActive;
	}

	HBBR_INLINE GameObject* GetGameObject() const {
		return _gameObject;
	}

	virtual void SetActive(bool newActive = true);

protected:

	virtual void GameObjectActiveChanged(bool gameObjectActive);

	virtual void Init();

	virtual void Update();

	virtual void ExecuteDestroy();

	bool _bActive;

	bool _bInit;

	class GameObject* _gameObject = NULL;
	
};