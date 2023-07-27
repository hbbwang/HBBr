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
	friend class PassManager;
public:
#if defined(_WIN32)
	HBBR_API VulkanRenderer(void* windowHandle, const char* rendererName);
#endif
	HBBR_API ~VulkanRenderer();

	/* Get current Frame buffer index. It is frequent use in the passes. */
	__forceinline static int GetCurrentFrameIndex() {
		return _currentFrameIndex;
	}

	/* Current swapchain present image index. It is not used very often. */
	__forceinline int GetSwapchinImageIndex() {
		return _swapchainIndex;
	}

	__forceinline bool IsRendererWantRelease() {
		return _bRendererRelease;
	}

	__forceinline class PassManager* GetPassManager() {
		return _passManager.get();
	}

	__forceinline std::shared_ptr<Texture> GetSwapchainImage() {
		return _swapchainTextures[_currentFrameIndex];
	}

	/* Get swapchain imageViews for render attachments */
	__forceinline std::vector<VkImageView> GetSwapchainImageViews() {
		return _swapchainImageViews;
	}

	__forceinline VkExtent2D GetSurfaceSize()const {
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

	HBBR_API void RendererResize();

	HBBR_API void Release();

	void Init();

	//Renderer map
	static std::map<HString, VulkanRenderer*> _renderers;

private:

	HString _rendererName;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkSurfaceFormatKHR _surfaceFormat{};

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	VkExtent2D _surfaceSize{};

	//std::vector<VkImage> _swapchainImages;

	//std::vector<VkImageView> _swapchainImageViews;

	std::vector<std::shared_ptr<Texture>> _swapchainTextures;

	std::vector<VkImageView>_swapchainImageViews;

	std::vector<VkSemaphore> _presentSemaphore;

	static uint32_t _currentFrameIndex;

	uint32_t _swapchainIndex;

	bool _bRendererRelease;

	bool _bInit;

	bool _bResize;

	//Passes
	std::unique_ptr<class PassManager> _passManager;
};