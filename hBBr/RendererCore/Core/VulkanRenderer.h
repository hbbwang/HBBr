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
	HBBR_API VulkanRenderer(void* windowHandle, const char* rendererName);
#endif
	HBBR_API ~VulkanRenderer();

	/* Frame buffer index */
	__forceinline static int GetCurrentFrameIndex() {
		return _currentFrameIndex;
	}

	__forceinline int GetSwapchinImageIndex() {
		return _swapchainIndex;
	}

	__forceinline bool IsRendererWantRelease() {
		return _bRendererRelease;
	}

	__forceinline bool IsRendererWantResize() {
		return _bRendererResize;
	}

	__forceinline class PassManager* GetPassManager() {
		return _passManager.get();
	}

	__forceinline std::shared_ptr<Texture> GetSwapchainImage() {
		return _swapchainTextures[_currentFrameIndex];
	}

	__forceinline VkExtent2D GetSurfaceSize() {
		return _surfaceSize;
	}

	__forceinline VkSemaphore GetPresentSemaphore() {
		return _presentSemaphore[_currentFrameIndex];
	}

	__forceinline bool IsInit() {
		return _bInit;
	}

	__forceinline HString GetName() {
		return _rendererName;
	}

	/* 帧渲染函数 */
	HBBR_API void Render();

	/* 检查Swapchain是否过期(一般是因为窗口大小改变了,和swapchain初始化的大小不一致导致的) */
	HBBR_API void CheckSwapchainOutOfData();

	HBBR_API void ResetWindowSize(uint32_t width,uint32_t height);

	void Init();

	//Renderer map
	static std::map<HString, VulkanRenderer*> _renderers;

private:

	void RendererResize();

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

	static uint32_t _currentFrameIndex;

	uint32_t _swapchainIndex;

	bool _bRendererRelease;

	bool _bRendererResize;

	bool _bRendering;

	bool _bInit;
	//Passes
	std::unique_ptr<class PassManager> _passManager;

	//multithread
	static std::unique_ptr<std::thread> _renderThread;

};