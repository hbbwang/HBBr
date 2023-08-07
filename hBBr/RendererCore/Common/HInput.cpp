#include "HInput.h"
#include "VulkanRenderer.h"
#include "FormMain.h"

std::vector<KeyCallBack> HInput::_keyRegisterDefault;
std::vector<KeyCallBack> HInput::_keyRegisterRepeat;

void HInput::KeyProcess(void* focusWindowHandle , KeyCode key, KeyMod mod, Action action)
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

void HInput::ClearInput()
{
	_keyRegisterDefault.clear();
}