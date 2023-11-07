#pragma once
//基础Compoennt组件类,组件都是附加在GameObject上的,结构体参数必须至少带有class GameObject* parent
#include "Common.h"
#include <vector>
#include <map>

enum ComponentPropertyType
{
	CPT_Float,
	CPT_Float2,
	CPT_Float3,
	CPT_Float4,
	CPT_TextInput,
	CPT_Bool,
};

struct ComponentProperty
{
	//指向参数的指针
	void*	valuePtr;
	//分类
	HString category;
	//类型
	ComponentPropertyType type;
};

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

	//Component Property Reflection Add.
	HBBR_INLINE virtual void InitProperties()
	{
		AddProperty("bActive", &_bActive, CPT_Bool, "");
	}

	void AddProperty(HString name, void* valuePtr, ComponentPropertyType type, HString category)
	{
		ComponentProperty pro;
		pro.valuePtr = valuePtr;
		pro.category = category;
		pro.type = type;
		_compProperties.emplace(name, pro);
	}

	//<displayName , component>
	std::map<HString, ComponentProperty> _compProperties;

	virtual void GameObjectActiveChanged(bool gameObjectActive);

	virtual void Init();

	virtual void Update();

	virtual void ExecuteDestroy();

	bool _bActive;

	bool _bInit;

	class GameObject* _gameObject = NULL;
	
};