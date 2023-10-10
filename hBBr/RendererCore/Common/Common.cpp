#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
#include "FormMain.h"
#include <SDL3/SDL.h>
void MessageOut(const char* msg, bool bExit, bool bMessageBox, const char* textColor)
{
	HString msgStr = msg;
	msgStr = "[hBBr]:" + msgStr;
	ConsoleDebug::print_endl(msgStr, textColor);
    if (bMessageBox)
    {
#if defined(_WIN32)
		//MessageBoxA(NULL, msg, "message", MB_ICONERROR);
		#if NDEBUG
		MessageBoxA(NULL, msg, "message", MB_ICONERROR);
		#else
		DE_ASSERT(0, msg);
		#endif

#else
	SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, "HBBr msg", msg, NULL);
	//fflush(stdout);
#endif
    }
	if (bExit)
	{
		//exit(EXIT_FAILURE);
		VulkanApp::AppQuit();
	}
}

