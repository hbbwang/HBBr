#include "Common.h"
#include "ConsoleDebug.h"

void MessageOut(const char* msg, bool bExit, bool bMessageBox, const char* textColor)
{
	ConsoleDebug::print_endl(msg, textColor);
#if defined(_WIN32)
	if (bMessageBox)
	{
		//MessageBoxA(NULL, msg, "message", MB_ICONERROR);
		//assert( 0 && msg);
		DE_ASSERT(0, msg);
	}
#else
	printf(msg);
	fflush(stdout);
#endif
	if (bExit)
		exit(EXIT_FAILURE);
}

