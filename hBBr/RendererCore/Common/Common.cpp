#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
#include "FormMain.h"
#include <SDL3/SDL.h>
#include "VulkanManager.h"
void wrapText(HString& msg, size_t lineWidth)
{
	size_t fontCount = 0;
	std::string str = msg.c_str();
	auto it = str.begin();
	while (1) {
		fontCount += lineWidth;
		if (str.length() > fontCount)
		{
			it += lineWidth;
			if (it != str.end())
			{
				str.insert(it, '\n');
			}
		}
		else
			break;
	}
	msg = str.c_str();
}

void MessageOut(HString msg, bool bExit, bool bMessageBox, const char* textColor)
{
	ConsoleDebug::print_endl(msg, textColor);
    if (bMessageBox && VulkanManager::_bDebugEnable)
    {
		#if IS_EDITOR
		const SDL_MessageBoxButtonData buttons[] = {
			{ 0, 0, "继续" },
			{ 0, 1, "中断" },
			{ 0, 2, "停止" },
		};

		wrapText(msg, 75);

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
		else if (buttonid == 2)
		{
			VulkanApp::AppQuit();
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
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, msg, nullptr);
	ConsoleDebug::print_endl(msg);
}

