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
#include "GameObject.h"
#include "Serializable.h"

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

#define AddProperty(_typeString,_name,_value,_bArray,_category,_sort,_condition,_readOnly)\
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
	pro.bReadOnly = _readOnly;\
	if (_compProperties.capacity() <= _compProperties.size())\
	{\
		if (_compProperties.capacity() < 200)\
			_compProperties.reserve(_compProperties.capacity() + 20);\
		else\
			_compProperties.reserve(_compProperties.capacity() * 2);\
	}\
	_compProperties.push_back(pro);\
	auto valueCompare = [](const ComponentProperty& p1, const ComponentProperty& p2)->bool {\
		return p1.sort < p2.sort;\
	};\
	std::sort(_compProperties.begin(), _compProperties.end(), valueCompare);\
}

struct AssetRef
{
	HString path = "";
	HString displayName = "";
	std::weak_ptr<AssetInfoBase> assetInfo;
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

	bool bReadOnly = false;
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

	HBBR_API HBBR_INLINE void EnableKeyInput(bool bEnable)	{
		_bEnableKeyInput = bEnable;
	}

	HBBR_API HBBR_INLINE void EnableMouseInput(bool bEnable) {
		_bEnableMouseInput = bEnable;
	}

	//不是每帧执行,一般用于编辑器的对象数据刷新
	//编辑器Inspector如果改了组件的参数，是通过调用这个函数进行刷新
	HBBR_API virtual void UpdateData() {}

	HBBR_API virtual void Deserialization(nlohmann::json input);

	HBBR_API virtual nlohmann::json Serialization();

protected:

	HBBR_INLINE virtual void OnConstruction() {
		//Component Property Reflection Add.
		AddProperty("bool", "bActive", &_bActive, false, "Default", 0, "", false);
	}

	std::vector<KeyCallBack> _keys;
	std::vector<MouseCallBack> _mouses;
	std::vector<KeyCallBack> _keys_repeat;
	std::vector<MouseCallBack> _mouses_repeat;

	inline bool GetKey(KeyCode key) {
		if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->renderer != _renderer)
			return false;
		return FindRepeatKey(key) != nullptr;
	}

	inline bool GetKeyDown(KeyCode key) {
		if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->renderer != _renderer)
			return false;
		return FindDefaultKeyDown(key, Action::PRESS) != nullptr;
	}

	inline bool GetKeyUp(KeyCode key) {
		if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->renderer != _renderer)
			return false;
		return FindDefaultKey(key, Action::RELEASE) != nullptr;
	}

	inline bool GetMouse(MouseButton button) {
		if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->renderer != _renderer)
			return false;
		return FindRepeatMouse(button) != nullptr;
	}

	inline bool GetMouseDown(MouseButton button) {
		if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->renderer != _renderer)
			return false;
		return FindDefaultMouseDown(button, Action::PRESS) != nullptr;
	}

	inline bool GetMouseUp(MouseButton button) {
		if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->renderer != _renderer)
			return false;
		return FindDefaultMouse(button, Action::RELEASE) != nullptr;
	}

	inline void SetMousePos(glm::vec2 pos) 
	{
		if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->renderer != _renderer)
			return;
		return HInput::SetMousePos(pos);
	}

	inline KeyCallBack* FindDefaultKey(KeyCode key, Action action)
	{
		auto it = std::find_if(_keys.begin(), _keys.end(), [key, action](KeyCallBack& callback) {
			return callback.key == key && callback.action == action;
			});
		if (it != _keys.end())
			return &(*it);
		else
			return nullptr;
	}

	inline KeyCallBack* FindDefaultKeyDown(KeyCode key, Action action)
	{
		auto it = std::find_if(_keys_repeat.begin(), _keys_repeat.end(), [key, action](KeyCallBack& callback) {
			return callback.key == key && callback.action == action && callback.bKeyDown == true;
			});
		if (it != _keys_repeat.end())
			return &(*it);
		else
			return nullptr;
	}

	inline KeyCallBack* FindRepeatKey(KeyCode key)
	{
		auto it = std::find_if(_keys_repeat.begin(), _keys_repeat.end(), [key](KeyCallBack& callback) {
			return callback.key == key ;
			});
		if (it != _keys_repeat.end())
			return &(*it);
		else
			return nullptr;
	}

	//
	inline MouseCallBack* FindDefaultMouse(MouseButton mouse, Action action)
	{
		auto it = std::find_if(_mouses.begin(), _mouses.end(), [mouse, action](MouseCallBack& callback) {
			return callback.button == mouse && callback.action == action ;
			});
		if (it != _mouses.end())
			return &(*it);
		else
			return nullptr;
	}

	inline MouseCallBack* FindDefaultMouseDown(MouseButton mouse, Action action)
	{
		auto it = std::find_if(_mouses_repeat.begin(), _mouses_repeat.end(), [mouse, action](MouseCallBack& callback) {
			return callback.button == mouse && callback.action == action && callback.bMouseDown == true;
			});
		if (it != _mouses_repeat.end())
			return &(*it);
		else
			return nullptr;
	}

	inline MouseCallBack* FindRepeatMouse(MouseButton mouse)
	{
		auto it = std::find_if(_mouses_repeat.begin(), _mouses_repeat.end(), [mouse](MouseCallBack& callback) {
			return callback.button == mouse ;
			});
		if (it != _mouses_repeat.end())
		{
			return &(*it);
		}
		else
			return nullptr;
	}

	virtual void KeyInput(KeyCode key, KeyMod mod, Action action) {

	}

	virtual void MouseInput(MouseButton mouse, Action action) {

	}

	//<displayName , component>
	std::vector<ComponentProperty> _compProperties;

	virtual void GameObjectActiveChanged(bool gameObjectActive);

	virtual void Init();

	virtual void Update();

	virtual void ExecuteDestroy();

	bool _bEnableKeyInput = false;

	bool _bEnableMouseInput = false;

	bool _bActive;

	bool _bInit;

	HString _typeName = "Component";

	class GameObject* _gameObject = nullptr;
	
	class VulkanRenderer* _renderer = nullptr;

	class World* _world = nullptr;


	static HString PropertyValueToString(ComponentProperty& p);

	static void StringToPropertyValue(ComponentProperty& p, HString& valueStr);

	private:
		void CompUpdate();

};