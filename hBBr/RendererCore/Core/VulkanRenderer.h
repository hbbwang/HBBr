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

	__forceinline bool IsRendererWantRelease() {
		return _bRendererRelease;
	}

	__forceinline class PassManager* GetPassManager() {
		return _passManager.get();
	}

	__forceinline VkSurfaceFormatKHR GetSurfaceFormat()const {
		return _surfaceFormat;
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

	__forceinline VkSemaphore GetSubmitSemaphore() {
		return _queueSubmitSemaphore[_currentFrameIndex];
	}

	__forceinline bool IsInit() {
		return _bInit;
	}

	__forceinline HString GetName() {
		return _rendererName;
	}

	__forceinline VkCommandBuffer GetCommandBuffer()const{
		return _cmdBuf[_currentFrameIndex];
	}

	/* 帧渲染函数 */
	HBBR_API void Render();

	HBBR_API void RendererResize(uint32_t w,uint32_t h);

	HBBR_API void Release();

	void Init();

	//Renderer map
	static std::map<HString, VulkanRenderer*> _renderers;

private:

	void Resizing(bool bForce = false);

	HString _rendererName;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkSurfaceFormatKHR _surfaceFormat{};

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	VkExtent2D _surfaceSize{};

	VkExtent2D _windowSize{};

	std::vector<VkImage>	_swapchainImages;

	std::vector<VkImageView>_swapchainImageViews;

	std::vector<VkSemaphore> _presentSemaphore;

	std::vector<VkSemaphore> _queueSubmitSemaphore;

	static uint32_t _currentFrameIndex;

	bool _bRendererRelease;

	bool _bInit;

	bool _bResize;

	std::vector<VkCommandBuffer> _cmdBuf;

	//Passes
	std::unique_ptr<class PassManager> _passManager;
};