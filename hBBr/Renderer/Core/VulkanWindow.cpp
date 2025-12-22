#include "VulkanWindow.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "../Common/Common.h"

#ifdef _WIN32
#pragma comment(lib, "SDL3-static.lib")
#endif

VulkanWindow::VulkanWindow(int width, int height, const char* title)
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

VulkanWindow::~VulkanWindow()
{
}

std::string VulkanWindow::GetTitle()
{
	return SDL_GetWindowTitle(WindowHandle);
}

void VulkanWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(WindowHandle, title);
}

void VulkanWindow::SetFocus()
{
	SDL_RaiseWindow(WindowHandle);
}

void VulkanWindow::Update_MainThread()
{
	//Init Swapchain
	if (!Swapchain)
	{
		Swapchain.reset(new VulkanSwapchain(WindowHandle));
	}
	Swapchain->Update_MainThread();
}

void VulkanWindow::Update_RenderThead()
{
	if (Swapchain)
	{
		Swapchain->Update_RenderThread();
	}
}