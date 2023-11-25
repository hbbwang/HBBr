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
	{
		_keyRegisterDefault.push_back(callBack);
		if (action == Action::RELEASE)
		{
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
		_mouseRegisterDefault.push_back(callBack);
		if (action == Action::RELEASE)
		{
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
			_mouseRegisterRepeat.push_back(callBack);
		}
	}
}