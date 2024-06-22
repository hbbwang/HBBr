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
		VulkanRenderer::RemoveKeyInput(this);
	}
	if (_bEnableMouseInput)
	{
		VulkanRenderer::RemoveMouseInput(this);
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
	if (_bEnableKeyInput)
	{
		auto func = [this](VulkanRenderer* ptr, KeyCode key, KeyMod mod, Action action)
			{		
				if (VulkanApp::GetFocusForm() == nullptr ||  VulkanApp::GetFocusForm()->renderer != ptr)
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
		VulkanRenderer::SetupKeyInput(this, func);
	}
	if (_bEnableMouseInput)
	{
		auto func = [this](VulkanRenderer* ptr, MouseButton mouse, Action action)
			{
				if (VulkanApp::GetFocusForm() == nullptr || VulkanApp::GetFocusForm()->renderer != ptr)
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
		VulkanRenderer::SetupMouseInput(this, func);
	}
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
	auto it = std::remove_if(this->GetGameObject()->_comps.begin(), this->GetGameObject()->_comps.end(), [this](Component*& comp) {
		return comp == this;
		});
	if (it != this->GetGameObject()->_comps.end())
	{
		this->GetGameObject()->_comps.erase(it);
		delete this;
	}
}
