#pragma once
#include "VulkanManager.h"
#include <memory>

class HBBR_API VulkanRenderer
{
public:
#if defined(_WIN32)
	VulkanRenderer(void* windowHandle, bool bDebug);
#endif
	~VulkanRenderer();

	__forceinline static VulkanManager* GetManager()
	{
		return _vulkanManager;
	}

	__forceinline VkSurfaceKHR GetSurface()const{
		return _surface;
	}

private:
	
	static VulkanManager* _vulkanManager;

	VkSurfaceKHR _surface;

	VkSurfaceFormatKHR _surfaceFormat{};

	VkSwapchainKHR _swapchain;
};