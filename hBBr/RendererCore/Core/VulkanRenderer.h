#pragma once
#include "VulkanManager.h"
#include <memory>

class Texture;
class FrameBufferTexture;

class VulkanRenderer
{
public:
#if defined(_WIN32)
	HBBR_API VulkanRenderer(void* windowHandle, bool bDebug);
#endif
	HBBR_API ~VulkanRenderer();

	HBBR_API __forceinline static VulkanManager* GetManager()
	{
		return _vulkanManager;
	}

	/* Frame buffer index */
	HBBR_API __forceinline static int GetCurrentFrameIndex() {
		return _currentFrameIndex;
	}

	/* 帧渲染函数 */
	HBBR_API void Render();

	/* 检查Swapchain是否过期(一般是因为窗口大小改变了,和swapchain初始化的大小不一致导致的) */
	HBBR_API void CheckSwapchainOutOfData();

	HBBR_API void ResetWindowSize(uint32_t width,uint32_t height);

private:
	
	static VulkanManager* _vulkanManager;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkSurfaceFormatKHR _surfaceFormat{};

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	VkExtent2D _surfaceSize{};

	VkExtent2D _windowSize{};

	std::vector<VkImage> _swapchainImages;

	std::vector<VkImageView> _swapchainImageViews;

	std::shared_ptr<FrameBufferTexture> _swapchainRenderTarget;

	std::vector<VkSemaphore> _presentSemaphore;

	static uint32_t _currentFrameIndex;

	bool bRendererRelease;

};