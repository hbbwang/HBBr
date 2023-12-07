#include"Component.h"
#include "GameObject.h"
#include "Asset/World.h"
#include "Asset/Level.h"
#include "VulkanRenderer.h"
#include "ModelData.h"
#include "Texture.h"
#include "Material.h"
Component::Component(GameObject* parent)
{
	_bActive = true;
	_gameObject = parent;
	_world = _gameObject->_level->GetWorld();
	_renderer = _world->GetRenderer();
}

Component::~Component()
{

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
}

void Component::Update()
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
}

void Component::ExecuteDestroy()
{

}

void Component::Destroy()
{
	SetActive(false);
	this->ExecuteDestroy();
	auto it = std::remove_if(this->GetGameObject()->_comps.begin(), this->GetGameObject()->_comps.end(), [this](Component*& comp) {
		return comp == this;
		});
	if (it != this->GetGameObject()->_comps.end())
	{
		this->GetGameObject()->_comps.erase(it);
		delete this;
	}
}

HString Component::AnalysisPropertyValue(ComponentProperty& p)
{
	//Base type
	if (p.type == typeid(bool).name())
	{
		auto value = (bool*)p.value;
		return (*value == true) ? "1" : "0";
	}
	else if (p.type == typeid(int).name())
	{
		auto value = (int*)p.value;
		return HString::FromInt(*value);
	}
	else if (p.type == typeid(uint64_t).name())
	{
		auto value = (uint64_t*)p.value;
		return HString::FromSize_t(*value);
	}
	else if (p.type == typeid(uint32_t).name() || p.type == typeid(uint16_t).name() || p.type == typeid(uint8_t).name())
	{
		auto value = (uint32_t*)p.value;
		return HString::FromUInt(*value);
	}
	else if (p.type == typeid(float).name())
	{
		auto value = (float*)p.value;
		return HString::FromFloat(*value);
	}
	else if (p.type == typeid(HString).name())
	{
		auto value = (HString*)p.value;
		return HString(*value);
	}
	//Asset type
	else if (p.type == typeid(ModelData).name())
	{
		auto value = (std::weak_ptr<ModelData>*)p.value;
		return value->lock()->_assetInfo->guid.str().c_str();
	}
	else if (p.type == typeid(Texture).name())
	{
		auto value = (std::weak_ptr<Texture>*)p.value;
		return value->lock()->_assetInfo->guid.str().c_str();
	}
	else if (p.type == typeid(Material).name())
	{
		auto value = (std::weak_ptr<Material>*)p.value;
		return value->lock()->_assetInfo->guid.str().c_str();
	}

	return "";
}
