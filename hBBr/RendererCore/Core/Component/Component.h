#pragma once
//基础Compoennt组件类,组件都是附加在GameObject上的,结构体参数必须至少带有class GameObject* parent
#include "Common.h"
#include <vector>
#include <map>
#include <functional>
#include <typeinfo>
#include <any>
#include "HInput.h"
#include "AssetObject.h"

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

#define AddProperty(_class,_name,_value,_bArray,_category,_sort)\
{\
	ComponentProperty pro;\
	if (std::is_base_of<AssetObject, _class>::value)\
	{\
		pro.bAsset = true;\
	}\
	pro.name = _name;\
	pro.value = _value;\
	pro.type = #_class;\
	pro.type_runtime = typeid(_class).name();\
	pro.category = _category;\
	pro.bArray = _bArray;\
	pro.sort = _sort;\
	_compProperties.push_back(pro);\
	auto valueCompare = [](const ComponentProperty& p1, const ComponentProperty& p2)->bool {\
		return p1.sort < p2.sort;\
	};\
	std::sort(_compProperties.begin(), _compProperties.end(), valueCompare);\
}

struct ComponentProperty
{
	HString name;
	//指向参数的指针
	void* value = nullptr;
	//分类
	HString category;
	//排序
	int sort = 32;
	//Type String
	HString type = "void";
	//runtime type id
	std::string type_runtime = "";
	bool bArray = false;
	bool bAsset = false;
};

class Component
{
	friend class GameObject;
	friend class World;
	friend class Level;
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

	HBBR_API HBBR_INLINE std::vector<ComponentProperty> GetProperties() const {
		return _compProperties;
	}

	HBBR_API HBBR_INLINE ComponentProperty* FindProperty(HString name) const {
		auto it = std::find_if(_compProperties.begin(), _compProperties.end(), [name](const ComponentProperty& p) {
			return name == p.name;
		});
		if (it != _compProperties.end())
		{
			return (it._Ptr);
		}
		return nullptr;
	}

	HBBR_API HBBR_INLINE HString GetComponentName() const {
		return _typeName;
	}

	HBBR_API HBBR_INLINE class World* GetWorld() const {
		return _world;
	}
	
protected:

	HBBR_INLINE virtual void OnConstruction() {
		//Component Property Reflection Add.
		AddProperty(bool, "bActive", &_bActive, false, "Default", 0);
	}

	static HString AnalysisPropertyValue(ComponentProperty& p);

	//<displayName , component>
	std::vector<ComponentProperty> _compProperties;

	virtual void GameObjectActiveChanged(bool gameObjectActive);

	virtual void Init();

	virtual void Update();

	virtual void ExecuteDestroy();

	//input fallback
	inline bool GetKey(KeyCode key) {
		return HInput::GetKey(key,_renderer);
	}

	inline bool GetKeyDown(KeyCode key) {
		return HInput::GetKeyDown(key, _renderer);
	}

	inline bool GetKeyUp(KeyCode key) {
		return HInput::GetKeyUp(key, _renderer);
	}

	inline bool GetMouse(MouseButton button) {
		return HInput::GetMouse(button, _renderer);
	}

	inline bool GetMouseDown(MouseButton button) {
		return HInput::GetMouseDown(button, _renderer);
	}

	inline bool GetMouseUp(MouseButton button) {
		return HInput::GetMouseUp(button, _renderer);
	}

	inline glm::vec2 GetMousePos()
	{
		return HInput::GetMousePos();
	}

	inline glm::vec2 GetMousePosClient()
	{
		return HInput::GetMousePosClient();
	}

	bool _bActive;

	bool _bInit;

	HString _typeName = "Component";

	class GameObject* _gameObject = nullptr;
	
	class VulkanRenderer* _renderer = nullptr;

	class World* _world = nullptr;
};