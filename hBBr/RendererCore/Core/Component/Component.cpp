#include"Component.h"
#include "GameObject.h"
#include "Asset/World.h"
#include "Asset/Level.h"
#include "VulkanRenderer.h"
#include "Model.h"
#include "Texture2D.h"
#include "Material.h"
#include "HInput.h"

Component::Component(GameObject* parent)
{
	_bActive = true;
	_gameObject = parent;
	_world = _gameObject->_level->GetWorld();
	_renderer = _world->GetRenderer();
}

Component::~Component()
{
	if (_bEnableKeyInput)
	{
		_renderer->RemoveKeyInput(this);
	}
	if (_bEnableMouseInput)
	{
		_renderer->RemoveMouseInput(this);
	}
}

void Component::SetActive(bool newActive)
{
	_bActive = newActive;
	GameObjectActiveChanged(_gameObject->_bActive);
	if (!newActive)
	{

	}
}

void Component::GameObjectActiveChanged(bool gameObjectActive)
{
}

void Component::Init()
{
	_bInit = true;
	//注册使用按键输入
	if (_bEnableKeyInput)
	{
		//正常情况下不会出现超过10个按键同时按下吧？
		_keys.reserve(10);
		_keys_repeat.reserve(10);
		auto func = [this](VulkanRenderer* ptr, KeyCode key, KeyMod mod, Action action)
			{		
				if (ptr->HasInputFocus())
					return;
				KeyCallBack callBack;
				callBack.key = key;
				callBack.action = action;
				callBack.mod = mod;
				if (action == Action::RELEASE)
				{
					callBack.bKeyDown = false;

					if (std::find_if(_keys.begin(), _keys.end(), [&](KeyCallBack& item) {
						return callBack.key == item.key && callBack.action == item.action && callBack.mod == item.mod;
						}) == _keys.end())
					{
						_keys.push_back(callBack);
					}

					for (int i = 0; i < _keys_repeat.size(); i++)
					{
						if (_keys_repeat[i].key == key)
						{
							_keys_repeat.erase(_keys_repeat.begin() + i);
							i -= 1;
							if (_keys_repeat.size() <= 0)
								break;
						}
					}
				}
				else if (action == Action::PRESS)
				{
					callBack.bKeyDown = true;
					if (std::find_if(_keys_repeat.begin(), _keys_repeat.end(), [&](KeyCallBack& item) {
						return callBack.key == item.key && callBack.action == item.action && callBack.mod == item.mod;
						}) == _keys_repeat.end())
					{
						_keys_repeat.push_back(callBack);
					}
				}

				KeyInput(key, mod, action);
			};
		_renderer->SetupKeyInput(this, func);
	}
	if (_bEnableMouseInput)
	{
		_mouses.reserve(5);
		_mouses_repeat.reserve(5);
		auto func = [this](VulkanRenderer* ptr, MouseButton mouse, Action action)
			{
				if (ptr->HasInputFocus())
					return;
				MouseCallBack callBack;
				callBack.button = mouse;
				callBack.action = action;
				{
					if (action == Action::RELEASE)
					{
						callBack.bMouseDown = false;

						if (std::find_if(_mouses.begin(), _mouses.end(), [&](MouseCallBack& item) {
							return callBack.button == item.button && callBack.action == item.action;
							}) == _mouses.end())
						{
							_mouses.push_back(callBack);
						}

						for (int i = 0; i < _mouses_repeat.size(); i++)
						{
							if (_mouses_repeat[i].button == mouse)
							{
								_mouses_repeat.erase(_mouses_repeat.begin() + i);
								i -= 1;
								if (_mouses_repeat.size() <= 0)
									break;
							}
						}
					}
					else if (action == Action::PRESS)
					{
						callBack.bMouseDown = true;
						if (std::find_if(_mouses_repeat.begin(), _mouses_repeat.end(), [&](MouseCallBack& item) {
							return callBack.button == item.button && callBack.action == item.action;
							}) == _mouses_repeat.end())
						{
							_mouses_repeat.push_back(callBack);
						}
					}
				}

				MouseInput(mouse, action);
			};
		_renderer->SetupMouseInput(this, func);
	}
}

void Component::Deserialization(nlohmann::json input)
{
	nlohmann::json compPros = input["Properties"];
	for (auto& p : compPros.items())
	{
		std::string proName = p.key();
		std::string proType = p.value()["Type"];
		std::string proValue = p.value()["Value"];
		for (auto& pp : _compProperties)
		{
			if (pp.name == proName && pp.type == proType)
			{
				Component::StringToPropertyValue(pp, proValue);
				break;
			}
		}
	}
}

nlohmann::json Component::Serialization()
{
	nlohmann::json sub;
	nlohmann::json subPros;
	sub["Class"] = GetComponentName();
	//Variables
	for (auto& p : _compProperties)
	{
		nlohmann::json subPro;
		auto valueStr = Component::PropertyValueToString(p);
		if (valueStr.length() > 0)
		{
			subPro["Type"] = p.type;
			subPro["Value"] = valueStr;
			subPros[p.name.c_str()] = subPro;
		}
	}
	sub["Properties"] = subPros;
	return sub;
}

bool Component::GetKey(KeyCode key)
{
	if (_renderer->HasInputFocus())
		return false;
	return FindRepeatKey(key) != nullptr;
}

bool Component::GetKeyDown(KeyCode key)
{
	if (_renderer->HasInputFocus())
		return false;
	return FindDefaultKeyDown(key, Action::PRESS) != nullptr;
}

bool Component::GetKeyUp(KeyCode key)
{
	if (_renderer->HasInputFocus())
		return false;
	return FindDefaultKey(key, Action::RELEASE) != nullptr;
}

bool Component::GetMouse(MouseButton button)
{
	if (_renderer->HasInputFocus())
		return false;
	return FindRepeatMouse(button) != nullptr;
}

bool Component::GetMouseDown(MouseButton button)
{
	if (_renderer->HasInputFocus())
		return false;
	return FindDefaultMouseDown(button, Action::PRESS) != nullptr;
}

inline bool Component::GetMouseUp(MouseButton button)
{
	if (_renderer->HasInputFocus())
		return false;
	return FindDefaultMouse(button, Action::RELEASE) != nullptr;
}

void Component::SetMousePos(glm::vec2 pos)
{
	if (_renderer->HasInputFocus())
		return;
	return HInput::SetMousePos(pos);
}

void Component::CompUpdate()
{
	if (_bActive)
	{
		if (!_bInit)//Init
		{
			Init();
		}
		else
		{

		}
	}
	Update();
	_keys.clear();
	 _mouses.clear();

	 for (auto& i : _keys_repeat)
	 {
		 i.bKeyDown = false;
	 }
	 for (auto& i : _mouses_repeat)
	 {
		 i.bMouseDown = false;
	 }
}

void Component::Update()
{
	if (_bActive)
	{
	}
}

void Component::ExecuteDestroy()
{

}

void Component::Destroy()
{
	SetActive(false);
	this->ExecuteDestroy();
}

std::string Component::PropertyValueToString(ComponentProperty& p)
{
	if (StringTool::IsEqual(p.type, "bool", false))
	{
		if (p.bArray)
		{
			auto value = (std::vector<bool>*)p.value;
			std::string result;
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
	else if (StringTool::IsEqual(p.type, "float", false))
	{
		if (p.bArray)
		{
			auto value = (std::vector<float>*)p.value;
			std::string result;
			for (auto i : *value) {
				result += StringTool::FromFloat(i) + ";";
			}
			return result;
		}
		else
		{
			auto value = (float*)p.value;
			return StringTool::FromFloat(*value);
		}
	}
	else if (StringTool::IsEqual(p.type, "Vector2", false))
	{
		if (p.bArray)
		{
			auto value = (std::vector<glm::vec2>*)p.value;
			std::string result;
			for (auto i : *value) {
				result += StringTool::FromVec2(i) + ";";
			}
			return result;
		}
		else
		{
			auto value = (glm::vec2*)p.value;
			return StringTool::FromVec2(*value);
		}
	}
	else if (StringTool::IsEqual(p.type, "Vector3", false))
	{
		if (p.bArray)
		{
			auto value = (std::vector<glm::vec3>*)p.value;
			std::string result;
			for (auto i : *value) {
				result += StringTool::FromVec3(i) + ";";
			}
			return result;
		}
		else
		{
			auto value = (glm::vec3*)p.value;
			return StringTool::FromVec3(*value);
		}
	}
	else if (StringTool::IsEqual(p.type, "Vector4", false))
	{
		if (p.bArray)
		{
			auto value = (std::vector<glm::vec4>*)p.value;
			std::string result;
			for (auto i : *value) {
				result += StringTool::FromVec4(i) + ";";
			}
			return result;
		}
		else
		{
			auto value = (glm::vec4*)p.value;
			return StringTool::FromVec4(*value);
		}
	}
	else if (StringTool::IsEqual(p.type, "AssetRef", false))
	{
		if (p.bArray)
		{
			auto value = (std::vector<AssetRef>*)p.value;
			std::string result;
			for (auto i : *value) {
				result += i.path + ";";
			}
			return result;
		}
		else
		{
			auto value = (AssetRef*)p.value;
			return value->path;
		}
	}
	return "";
}


void Component::StringToPropertyValue(ComponentProperty& p, std::string& valueStr)
{
	if (StringTool::IsEqual(p.type, "bool", false))
	{
		if (p.bArray)
		{
			auto bools = StringTool::split(valueStr, ";");
			auto valuePtr = ((std::vector<bool>*)p.value);
			valuePtr->resize(bools.size());
			for (int i = 0; i < bools.size(); i++)
			{
				valuePtr->push_back(StringTool::ToInt(bools[i]) == 1 ? true : false);
			}
		}
		else
		{
			*((bool*)p.value) = StringTool::ToInt(valueStr) == 1 ? true : false;
		}
	}
	else if (StringTool::IsEqual(p.type, "float", false))
	{
		if (p.bArray)
		{
			auto values = StringTool::split(valueStr, ";");
			auto valuePtr = ((std::vector<float>*)p.value);
			valuePtr->resize(values.size());
			for (int i = 0; i < values.size(); i++)
			{
				valuePtr->push_back((float)StringTool::ToDouble(values[i]));
			}
		} 
		else
		{
			*((float*)p.value) = (float)StringTool::ToDouble(valueStr);
		}
	}
	else if (StringTool::IsEqual(p.type, "Vector2", false))
	{
		if (p.bArray)
		{
			auto values = StringTool::split(valueStr, ";");
			auto valuePtr = ((std::vector<glm::vec2>*)p.value);
			valuePtr->resize(values.size());
			for (int i = 0; i < values.size(); i++)
			{
				valuePtr->push_back(StringTool::ToVec2(values[i]));
			}
		}
		else
		{
			*((glm::vec2*)p.value) = StringTool::ToVec2(valueStr);
		}
	}
	else if (StringTool::IsEqual(p.type, "Vector3", false))
	{
		if (p.bArray)
		{
			auto values = StringTool::split(valueStr, ";");
			auto valuePtr = ((std::vector<glm::vec3>*)p.value);
			valuePtr->resize(values.size());
			for (int i = 0; i < values.size(); i++)
			{
				valuePtr->push_back(StringTool::ToVec3(values[i]));
			}
		}
		else
		{
			*((glm::vec3*)p.value) = StringTool::ToVec3(valueStr);
		}
	}
	else if (StringTool::IsEqual(p.type, "Vector4", false))
	{
		if (p.bArray)
		{
			auto values = StringTool::split(valueStr, ";");
			auto valuePtr = ((std::vector<glm::vec4>*)p.value);
			valuePtr->resize(values.size());
			for (int i = 0; i < values.size(); i++)
			{
				valuePtr->push_back(StringTool::ToVec4(values[i]));
			}
		}
		else
		{
			*((glm::vec4*)p.value) = StringTool::ToVec4(valueStr);
		}
	}
	else if (StringTool::IsEqual(p.type, "AssetRef", false))
	{
		if (p.bArray)
		{
			auto values = StringTool::split(valueStr, ";");
			auto valuePtr = ((std::vector<AssetRef>*)p.value);
			valuePtr->resize(values.size());
			for (int i = 0; i < values.size(); i++)
			{
				AssetRef newAssetPath;
				newAssetPath.path = values[i];
				valuePtr->at(i) = (newAssetPath);
			}
		}
		else
		{
			((AssetRef*)p.value)->path = valueStr;
		}
	}

}