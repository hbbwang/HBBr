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

	HBBR_API __forceinline VkSurfaceKHR GetSurface()const{
		return _surface;
	}

	HBBR_API __forceinline static int GetSwapchainBufferIndex() {
		return _swapchainBufferIndex;
	}

	/* 帧渲染函数 */
	HBBR_API void Render();

private:
	
	static VulkanManager* _vulkanManager;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkSurfaceFormatKHR _surfaceFormat{};

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	VkExtent2D _surfaceSize{};

	std::vector<VkImage> _swapchainImages;

	std::vector<VkImageView> _swapchainImageViews;

	std::vector<VkFramebuffer> _swapchainFrameBuffers;

	std::shared_ptr<FrameBufferTexture> _swapchainRenderTarget;

	static uint32_t _swapchainBufferIndex;
};