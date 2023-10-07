#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
#include "FormMain.h"
#include "GLFWInclude.h"
void MessageOut(const char* msg, bool bExit, bool bMessageBox, const char* textColor)
{
	HString msgStr = msg;
	ConsoleDebug::print_endl(msgStr, textColor);
#if defined(_WIN32)
	if (bMessageBox)
	{
		//MessageBoxA(NULL, msg, "message", MB_ICONERROR);
		#if NDEBUG
		MessageBoxA(NULL, msg, "message", MB_ICONERROR);
		#else
		DE_ASSERT(0, msg);
		#endif
	}
#else
	printf("%s",msg);
	fflush(stdout);
	SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, "HBBr msg", msg, NULL);
#endif
	if (bExit)
	{
		//exit(EXIT_FAILURE);
		VulkanApp::AppQuit();
	}
}

