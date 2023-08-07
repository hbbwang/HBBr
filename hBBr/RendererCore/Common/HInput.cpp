#include "HInput.h"

using namespace HInput;

std::vector<KeyCallBack> Input::_keyRegister;

void HInput::Input::KeyProcess(KeyCode key, KeyMod mod, Action action)
{
	KeyCallBack callBack;
	callBack.key = key;
	callBack.action = action;
	callBack.mod = mod;
	_keyRegister.push_back(callBack);
}

void HInput::Input::ClearInput()
{
	_keyRegister.clear();
}
