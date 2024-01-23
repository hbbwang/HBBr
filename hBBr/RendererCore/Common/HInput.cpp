#include "HInput.h"
#include "VulkanRenderer.h"
#include "FormMain.h"

std::vector<KeyCallBack> HInput::_keyRegisterDefault;
std::vector<KeyCallBack> HInput::_keyRegisterRepeat;
std::vector<MouseCallBack> HInput::_mouseRegisterDefault;
std::vector<MouseCallBack> HInput::_mouseRegisterRepeat;

void HInput::KeyProcess(VulkanForm* focusWindowHandle , KeyCode key, KeyMod mod, Action action)
{
	KeyCallBack callBack;
	callBack.key = key;
	callBack.action = action;
	callBack.mod = mod;
	callBack.focusWindowHandle = focusWindowHandle;
	if (action == Action::RELEASE)
	{
		callBack.bKeyDown = false;

		if (std::find_if(_keyRegisterDefault.begin(), _keyRegisterDefault.end(), [&](KeyCallBack item) {
			return callBack.key == item.key && callBack.action == item.action && callBack.mod == item.mod;
			}) == _keyRegisterDefault.end())
		{
			_keyRegisterDefault.push_back(callBack);
		}

		for (int i = 0; i < _keyRegisterRepeat.size(); i++)
		{
			if (_keyRegisterRepeat[i].key == key)
			{
				_keyRegisterRepeat.erase(_keyRegisterRepeat.begin() + i);
				i -= 1;
				if (_keyRegisterRepeat.size() <= 0)
					break;
			}
		}
	}
	else if (action == Action::PRESS)
	{
		callBack.bKeyDown = true;
		if (std::find_if(_keyRegisterRepeat.begin(), _keyRegisterRepeat.end(), [&](KeyCallBack item) {
			return callBack.key == item.key && callBack.action == item.action && callBack.mod == item.mod;
			}) == _keyRegisterRepeat.end())
		{
			_keyRegisterRepeat.push_back(callBack);
		}
	}
}

void HInput::MouseProcess(VulkanForm* focusWindowHandle, MouseButton mouse, Action action)
{
	MouseCallBack callBack;
	callBack.button = mouse;
	callBack.action = action;
	callBack.focusWindowHandle = focusWindowHandle;
	{
		if (action == Action::RELEASE)
		{
			callBack.bMouseDown = false;

			if (std::find_if(_mouseRegisterDefault.begin(), _mouseRegisterDefault.end(), [&](MouseCallBack item) {
				return callBack.button == item.button && callBack.action == item.action;
				}) == _mouseRegisterDefault.end())
			{
				_mouseRegisterDefault.push_back(callBack);
			}

			for (int i = 0; i < _mouseRegisterRepeat.size(); i++)
			{
				if (_mouseRegisterRepeat[i].button == mouse)
				{
					_mouseRegisterRepeat.erase(_mouseRegisterRepeat.begin() + i);
					i -= 1;
					if (_mouseRegisterRepeat.size() <= 0)
						break;
				}
			}
		}
		else if (action == Action::PRESS)
		{
			callBack.bMouseDown = true;
			if (std::find_if(_mouseRegisterRepeat.begin(), _mouseRegisterRepeat.end(), [&](MouseCallBack item) {
				return callBack.button == item.button && callBack.action == item.action;
				}) == _mouseRegisterRepeat.end())
			{
				_mouseRegisterRepeat.push_back(callBack);
			}
		}
	}
}