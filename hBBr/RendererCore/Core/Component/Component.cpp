#include"Component.h"
#include "GameObject.h"
#include "Resource/World.h"
#include "VulkanRenderer.h"
Component::Component(GameObject* parent)
{
	_bActive = true;
	_gameObject = parent;
	_renderer = _gameObject->_scene->GetRenderer();
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

