#pragma once
//基础Compoennt组件类,组件都是附加在GameObject上的,结构体参数必须至少带有class GameObject* parent
#include "Common.h"
#include <vector>
#include <map>

#define COMPONENT_DEFINE(ComponentClassName)\
public:\
	HBBR_API ComponentClassName();\
	HBBR_API ComponentClassName(class GameObject* parent);

#define COMPONENT_IMPLEMENT(ComponentClassName)\
ComponentClassName::ComponentClassName() :Component() \
{\
	this->_typeName = #ComponentClassName; \
	GameObject::GetCompSpawnMap().emplace(this->_typeName, [](GameObject* obj)->ComponentClassName*\
		{\
		auto comp = obj->AddComponent<ComponentClassName>(); \
		return comp; \
		}); \
}\
ComponentClassName::ComponentClassName(class GameObject* parent) :Component(parent)\
{\
	this->_typeName = #ComponentClassName; \
	this->OnConstruction();\
}\
\
ComponentClassName  _component_construct_##ComponentClassName;

enum ComponentPropertyType
{
	CPT_Float,
	CPT_Float2,
	CPT_Float3,
	CPT_Float4,
	CPT_Resource,
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
	Component() {}

	Component(class GameObject* parent);

	virtual  ~Component();

	HBBR_API void Destroy();

	HBBR_API virtual void SetActive(bool newActive = true);

	HBBR_API HBBR_INLINE bool IsActive() const {
		return _bActive;
	}

	HBBR_API HBBR_INLINE GameObject* GetGameObject() const {
		return _gameObject;
	}

	HBBR_API HBBR_INLINE std::map<HString, ComponentProperty> GetProperties() const {
		return _compProperties;
	}

	HBBR_API HBBR_INLINE HString GetComponentName() const {
		return _typeName;
	}
	
protected:

	HBBR_INLINE virtual void OnConstruction() {
		//Component Property Reflection Add.
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

	HString _typeName = "Component";

	class GameObject* _gameObject = NULL;
	
};