#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
#include "FormMain.h"
#include <SDL3/SDL.h>
#include "VulkanManager.h"
void MessageOut(HString msg, bool bExit, bool bMessageBox, const char* textColor)
{
	ConsoleDebug::print_endl(msg, textColor);
    if (bMessageBox && VulkanManager::_bDebugEnable)
    {
		#if IS_EDITOR
		const SDL_MessageBoxButtonData buttons[] = {
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "继续" },
			{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "中断" },
		};
		const SDL_MessageBoxData messageboxdata = {
			SDL_MESSAGEBOX_WARNING,
			NULL,
			"断言",
			msg.c_str(),
			SDL_arraysize(buttons),
			buttons,
			NULL
		};
		int buttonid;
		SDL_ShowMessageBox(&messageboxdata, &buttonid);
		if (buttonid == 1)
		{
			throw std::runtime_error("Program Exception.");
		}
		#endif
    }
	if (bExit)
	{
		VulkanApp::AppQuit();
	}
}

void MsgBox(const char* title, const char* msg)
{
	SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, title, msg, nullptr);
	ConsoleDebug::print_endl(msg);
}

