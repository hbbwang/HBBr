#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
#include "FormMain.h"
#include <SDL3/SDL.h>
#include "VulkanManager.h"
void MessageOut(HString msg, bool bExit, bool bMessageBox, const char* textColor)
{
	//HString msgStr = msg.c_wstr(); 
	//msgStr = "[hBBr]:" + msgStr;
    if (bMessageBox && VulkanManager::_bDebugEnable)
    {
		SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "HBBr msg", msg.c_str(), nullptr);
    }
	ConsoleDebug::print_endl(msg, textColor);
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

