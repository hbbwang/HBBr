#pragma once
#include "SDLWindow.h"
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include "concurrentqueue.h"
#include"VulkanSwapchain.h"

class VulkanApp
{
public:
	HBBR_API static VulkanApp* Get()
	{
		if (!Instance)
			Instance.reset(new VulkanApp());
		return Instance.get();
	}
	HBBR_API void Release();
	HBBR_API void InitVulkanManager(bool bEnableDebug = false);
	HBBR_API class VkWindow* CreateVulkanWindow(int w, int h , const char* title);
	HBBR_API bool MainLoop();
	HBBR_API VkWindow* GetFocusWindow();
	HBBR_API VkWindow* GetWindowFromID(SDL_WindowID id);
	HBBR_API void EnqueueRenderFunc(std::function<void()>func);
	bool RenderLoop();
private:
	std::thread RenderThread;
	static std::unique_ptr<VulkanApp> Instance;
	std::vector<class VkWindow*> AllWindows;
	moodycamel::ConcurrentQueue<std::function<void()>> RenderThreadFuncs;
};

//判断是否主线程
thread_local extern bool bIsMianThread;
//判断是否渲染线程
thread_local extern bool bIsRenderThread;
//判断是否初始化(初始化之后才能进行真正的Loop)
thread_local extern bool bInitialize;