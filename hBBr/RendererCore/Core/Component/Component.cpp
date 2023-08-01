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

bool Component::Update()
{
	if (_bWantDestroy)
	{
		auto it = std::remove(_gameObject->_comps.begin(), _gameObject->_comps.end(), this);
		if (it != _gameObject->_comps.end())
		{
			_gameObject->_comps.erase(it);
			ExecuteDestroy();
			delete this;
		}
		return false;
	}
	else
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
	return true;
}

void Component::ExecuteDestroy()
{

}
