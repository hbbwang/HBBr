#include"Component.h"
#include "GameObject.h"
Component::Component(GameObject* parent)
{
	_bActive = true;
	_gameObject = parent;
}

Component::~Component()
{

}

void Component::SetActive(bool newActive)
{
	_bActive = newActive;
	if (!newActive)
	{

	}
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
