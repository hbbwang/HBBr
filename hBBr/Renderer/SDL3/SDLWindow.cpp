#include "SDLWindow.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "../Common/Common.h"

#ifdef _WIN32
#pragma comment(lib, "SDL3-static.lib")
#endif

VKWindow::VKWindow(int width, int height, const char* title)
{
	ConsoleDebug::printf_endl("Create a new vulkan window.");
	WindowHandle = SDL_CreateWindow(
		title,
		width,
		height,
		SDL_WINDOW_RESIZABLE  | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS  | 
		SDL_WINDOW_VULKAN
	);
	if (!WindowHandle)
	{
		ConsoleDebug::printf_endl_error("Create SDL3 Window failed : {}", SDL_GetError());
		return;
	}
	WindowID = SDL_GetWindowID(WindowHandle);
}

VKWindow::~VKWindow()
{

}

std::string VKWindow::GetTitle()
{
	return SDL_GetWindowTitle(WindowHandle);
}

void VKWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(WindowHandle, title);
}

void VKWindow::SetFocus()
{
	SDL_RaiseWindow(WindowHandle);
}