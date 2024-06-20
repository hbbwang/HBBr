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

#define AddProperty(_typeString,_name,_value,_bArray,_category,_sort,_condition)\
{\
	ComponentProperty pro;\
	pro.comp = this;\
	pro.condition = _condition;\
	pro.name = _name;\
	pro.value = _value;\
	pro.type = _typeString;\
	pro.category = _category;\
	pro.bArray = _bArray;\
	pro.sort = _sort;\
	_compProperties.push_back(pro);\
	auto valueCompare = [](const ComponentProperty& p1, const ComponentProperty& p2)->bool {\
		return p1.sort < p2.sort;\
	};\
	std::sort(_compProperties.begin(), _compProperties.end(), valueCompare);\
}

struct AssetRef
{
	HString path = "";
	AssetInfoBase* assetInfo = nullptr;
	std::function<void()> callBack = []() {};
};

struct ComponentProperty
{
	class Component* comp = nullptr;

	HString name;

	//指向参数的指针
	void* value = nullptr;

	//分类
	HString category;

	HString condition;

	//排序
	int sort = 32;

	//Type String
	HString type = "void";

	bool bArray = false;
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

	HBBR_API HBBR_INLINE class Transform* GetTransform() {
		return _gameObject->_transform;
	}

	//不是每帧执行,一般用于编辑器的对象数据刷新
	//编辑器Inspector如果改了组件的参数，是通过调用这个函数进行刷新
	HBBR_API virtual void UpdateData() {}

protected:

	HBBR_INLINE virtual void OnConstruction() {
		//Component Property Reflection Add.
		AddProperty("bool", "bActive", &_bActive, false, "Default", 0, "");
	}

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


	static HString PropertyValueToString(ComponentProperty& p)
	{
		if (p.type.IsSame("bool", false))
		{
			if (p.bArray)
			{
				auto value = (std::vector<bool>*)p.value;
				HString result;
				for (auto i : *value) {
					result += (i == true) ? "1;" : "0;";
				}
				return result;
			}
			else
			{
				auto value = (bool*)p.value;
				return (*value == true) ? "1" : "0";
			}
		}
		else 	if (p.type.IsSame("AssetRef", false))
		{
			if (p.bArray)
			{
				auto value = (std::vector<AssetRef>*)p.value;
				HString result;
				for (auto i : *value) {
					result += i.path + ";";
				}
			}
			else
			{
				auto value = (AssetRef*)p.value;
				return value->path;
			}
		}
		return "";
	}

	static void StringToPropertyValue(ComponentProperty& p , HString& valueStr)
	{
		if (p.type.IsSame("bool", false))
		{
			if (p.bArray)
			{
				auto bools = valueStr.Split(";");
				auto valuePtr = ((std::vector<bool>*)p.value);
				valuePtr->resize(bools.size());
				for (int i = 0; i < bools.size(); i++)
				{
					valuePtr->push_back(HString::ToInt(bools[i]) == 1 ? true : false);
				}
			}
			else
			{
				*((bool*)p.value) = HString::ToInt(valueStr) == 1 ? true : false;
			}
		}
		else 	if (p.type.IsSame("AssetRef", false))
		{
			if (p.bArray)
			{
				auto values = valueStr.Split(";");
				auto valuePtr = ((std::vector<AssetRef>*)p.value);
				valuePtr->resize(values.size());
				for (int i = 0; i < values.size(); i++)
				{
					AssetRef newAssetPath;
					newAssetPath.path = values[i];
					valuePtr->push_back(newAssetPath);
				}
			}
			else
			{
				((AssetRef*)p.value)->path = valueStr;
			}
		}

	}


};