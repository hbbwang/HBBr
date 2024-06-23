#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
#include "FormMain.h"
#include <SDL3/SDL.h>
#include "VulkanManager.h"
void MessageOut(HString msg, bool bExit, bool bMessageBox, const char* textColor)
{
	HString msgStr = msg.c_wstr();
	//msgStr = "[hBBr]:" + msgStr;
    if (bMessageBox && VulkanManager::_bDebugEnable)
    {
#if defined(_WIN32)
		//MessageBoxA(nullptr, msg, "message", MB_ICONERROR);
		#if NDEBUG
		MessageBoxA(0, msgStr.c_str(), "HBBr msg", 0);
		#else
		DE_ASSERT(0, msgStr.c_str());
		#endif
#else
	SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, "HBBr msg", msg, nullptr);
	//fflush(stdout);
#endif
    }
	else
	{
		ConsoleDebug::print_endl(msgStr, textColor);
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

