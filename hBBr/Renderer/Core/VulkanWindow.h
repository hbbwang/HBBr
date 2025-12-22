#pragma once
#include "../Common/Common.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "SDL3/SDL_keyboard.h"
#include "VulkanSwapchain.h"

#ifdef _WIN32
#pragma comment(lib,"./SDL3-static.lib")
#pragma comment(lib,"imm32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"version.lib")
#pragma comment(lib,"setupapi.lib")
#else
#include <SDL3/SDL_main.h>
#endif

#include <vector>

class VulkanWindow 
{
public:
	VulkanWindow(int width, int height, const char* title);
	~VulkanWindow();
	void Update_MainThread();
	void Update_RenderThead();
	struct SDL_Window* GetWindowHandle() const { return WindowHandle; }
	SDL_WindowID GetWindowID() const { return WindowID; }
	std::string GetTitle();
	void SetTitle(const char* title);
	void SetFocus();
private:
	struct SDL_Window* WindowHandle = nullptr;
	SDL_WindowID WindowID;
	std::unique_ptr<VulkanSwapchain> Swapchain;
};