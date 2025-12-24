#include "Common.h"
#include "ConsoleDebug.h"
#include <assert.h>
#include <SDL3/SDL.h>
void wrapText(std::string& msg, size_t lineWidth)
{
	size_t fontCount = 0;
	std::string str = msg.c_str();
	while (1) {
		fontCount += lineWidth;
		if (str.length() > fontCount)
		{
			str.insert(fontCount, "\n");
		}
		else
			break;
	}
	msg = str.c_str();
}

void MessageOut(std::string msg, bool bExit, bool bMessageBox, const char* textColor)
{
	ConsoleDebug::print_endl(msg, textColor);
    if (bMessageBox)
    {
		#if 1
		const SDL_MessageBoxButtonData buttons[] = {
			{ 0, 0, "继续" },
			{ 0, 1, "中断" },
			{ 0, 2, "停止" },
		};

		wrapText(msg, 75);

		std::string cmsg = (msg);

		const SDL_MessageBoxData messageboxdata = {
			SDL_MESSAGEBOX_WARNING,
			NULL,
			"断言",
			cmsg.c_str(),
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
			exit(0);
		}
		#endif
    }
	if (bExit)
	{
		exit(0);
	}
}

void MsgBox(const char* title, const char* msg)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, msg, nullptr);
	ConsoleDebug::print_endl(msg);
}

