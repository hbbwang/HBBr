#pragma once
#include "VulkanWindow.h"
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include "concurrentqueue.h" 

class VulkanApp
{
public:
	virtual ~VulkanApp();
	HBBR_API static VulkanApp* Get()
	{
		if (!Instance)
			Instance.reset(new VulkanApp());
		return Instance.get();
	}
	HBBR_API void ReleaseVulkanManager();
	HBBR_API void InitVulkanManager(bool bEnableDebug = false);
	HBBR_API class VulkanWindow* CreateVulkanWindow(int w, int h , const char* title);
	HBBR_API bool MainLoop();
	HBBR_API VulkanWindow* GetFocusWindow();
	HBBR_API VulkanWindow* GetWindowFromID(SDL_WindowID id);
	HBBR_API void DestroyWindow(VulkanWindow* window);
	HBBR_API void EnqueueRenderFunc(std::function<void()>func);
	bool RenderLoop();
private:
	std::thread RenderThread;
	static std::unique_ptr<VulkanApp> Instance;
	std::atomic<std::shared_ptr<std::vector<std::shared_ptr<VulkanWindow>>>> AllWindows;
	moodycamel::ConcurrentQueue<std::function<void()>> RenderThreadFuncs;
	bool bHasMainLoop = false;
};

//判断是否主线程
thread_local extern bool bIsMainThread;
//判断是否渲染线程
thread_local extern bool bIsRenderThread;
//判断是否初始化(初始化之后才能进行真正的Loop)
thread_local extern bool bInitialize;