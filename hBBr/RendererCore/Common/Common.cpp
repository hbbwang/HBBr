#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
#include "FormMain.h"
#include <SDL3/SDL.h>
#include "VulkanManager.h"
void MessageOut(const char* msg, bool bExit, bool bMessageBox, const char* textColor)
{
	HString msgStr = msg;
	//msgStr = "[hBBr]:" + msgStr;
    if (bMessageBox && VulkanManager::GetManager()->_bDebugEnable)
    {
#if defined(_WIN32)
		//MessageBoxA(NULL, msg, "message", MB_ICONERROR);
		#if NDEBUG
		MessageBoxA(0, msgStr.c_str(), "HBBr msg", 0);
		#else
		DE_ASSERT(0, msgStr.c_str());
		#endif
#else
	SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, "HBBr msg", msg, NULL);
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
	SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, title, msg, NULL);
	ConsoleDebug::print_endl(msg);
}

