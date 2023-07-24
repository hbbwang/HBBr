#pragma once
#include "VulkanManager.h"
#include <memory>
#include <mutex>
#include <thread>
#include <map>
class Texture;
class FrameBufferTexture;

class VulkanRenderer
{
public:
#if defined(_WIN32)
	HBBR_API VulkanRenderer(void* windowHandle, const char* rendererName, bool bDebug);
#endif
	HBBR_API ~VulkanRenderer();

	HBBR_API __forceinline static std::shared_ptr<VulkanManager> GetManager(){
		return _vulkanManager;
	}

	/* Frame buffer index */
	HBBR_API __forceinline static int GetCurrentFrameIndex() {
		return _currentFrameIndex;
	}

	HBBR_API __forceinline int GetSwapchinImageIndex() {
		return _swapchainIndex;
	}

	HBBR_API __forceinline bool IsRendererWantRelease() {
		return _bRendererRelease;
	}

	HBBR_API __forceinline bool IsRendererWantResize() {
		return _bRendererResize;
	}

	HBBR_API __forceinline class PassManager* GetPassManager() {
		return _passManager.get();
	}

	HBBR_API __forceinline std::shared_ptr<Texture> GetSwapchainImage() {
		return _swapchainTextures[(_currentFrameIndex + 1) % 3 ];
	}

	/* 帧渲染函数 */
	HBBR_API void Render();

	/* 检查Swapchain是否过期(一般是因为窗口大小改变了,和swapchain初始化的大小不一致导致的) */
	HBBR_API void CheckSwapchainOutOfData();

	HBBR_API void ResetWindowSize(uint32_t width,uint32_t height);

private:

	void RendererResize();

	static std::shared_ptr<VulkanManager> _vulkanManager;

	//Renderer map
	static std::map<HString, VulkanRenderer*>_renderers;

	HString _rendererName;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkSurfaceFormatKHR _surfaceFormat{};

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	VkExtent2D _surfaceSize{};

	VkExtent2D _windowSize{};

	//std::vector<VkImage> _swapchainImages;

	//std::vector<VkImageView> _swapchainImageViews;

	std::vector<std::shared_ptr<Texture>> _swapchainTextures;

	std::shared_ptr<FrameBufferTexture> _swapchainRenderTarget; 

	std::vector<VkSemaphore> _presentSemaphore;

	std::vector<VkCommandBuffer> _commandBuffers;

	static uint32_t _currentFrameIndex;

	uint32_t _swapchainIndex;

	bool _bRendererRelease;

	bool _bRendererResize;

	bool _bRendering;

	//Passes
	std::unique_ptr<class PassManager> _passManager;

	//multithread
	std::thread _renderThread;
};