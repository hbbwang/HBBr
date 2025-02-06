#pragma once
#include "VulkanManager.h"
#include "FormMain.h"
#include <map>

class VulkanSwapchain
{
	friend class VulkanRenderer;
	friend class VulkanApp;
public:
	HBBR_API VulkanSwapchain(VulkanForm* form);

	HBBR_API ~VulkanSwapchain();

	HBBR_API HBBR_INLINE VulkanForm* GetSwapchainForm() {
		return _form;
	}

	HBBR_API HBBR_INLINE SDL_Window* GetWindowHandle() {
		return _form->window;
	}

	HBBR_API HBBR_INLINE bool HasFocus() {
		return VulkanApp::IsWindowFocus(_form->window);
	}

	HBBR_INLINE VkSurfaceFormatKHR GetSurfaceFormat()const {
		return _surfaceFormat;
	}

	//窗口分辨率
	HBBR_API HBBR_INLINE VkExtent2D GetWindowSurfaceSize()const {
		return _surfaceSize;
	}

	/* Get current Frame buffer index. It is frequent use in the passes. */
	HBBR_API HBBR_INLINE uint32_t GetCurrentFrameIndex() {
		return _currentFrameIndex;
	}

	/* Get swapchain imageViews for render attachments */
	HBBR_INLINE std::vector<VkImageView> GetSwapchainImageViews() {
		return _swapchainImageViews;
	}

	HBBR_INLINE std::vector<VkImage> GetSwapchainImages() {
		return _swapchainImages;
	}

	HBBR_API HBBR_INLINE std::map<std::string, class VulkanRenderer*> GetRenderers() {
		return _renderers;
	}

	HBBR_API HBBR_INLINE class VulkanRenderer* GetMainRenderer() {
		return _renderers.begin()->second;
	}

	HBBR_API VulkanRenderer* CreateRenderer(std::string rendererName);

	HBBR_API void DestroyRenderer(std::string rendererName);

	HBBR_API void DestroyRenderer(VulkanRenderer* renderer);

	HBBR_INLINE VkSemaphore* GetAcquireSemaphore() {
		return &_acquireSemaphore[_currentFrameIndex];
	}

	HBBR_INLINE VkCommandBuffer& GetCommandBuffer() {
		return _cmdBuf[_currentFrameIndex];
	}

	HBBR_API void Release();

	VkSurfaceCapabilitiesKHR _surfaceCapabilities{};

	bool bResizeBuffer;

	bool ResizeBuffer();

#if IS_EDITOR

	HBBR_API HBBR_INLINE class ImguiPassEditor* GetEditorGuiPass()const {
		return _imguiPassEditor.get();
	}

#endif

private:

	void Update();

	void ResetResource();

	bool bInit = false;

	VulkanManager* _vulkanManager = nullptr;

	struct VulkanForm* _form = nullptr;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkSurfaceFormatKHR _surfaceFormat{};

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	VkExtent2D _surfaceSize{};

	VkExtent2D _cacheSurfaceSize{};

	std::vector<VkImage>	_swapchainImages;

	std::vector<VkImageView>_swapchainImageViews;

	uint32_t _currentFrameIndex;

	uint32_t _maxSwapchainImages = 0;

	std::vector<VkSemaphore> _queueSemaphore;

	std::vector<VkSemaphore> _acquireSemaphore;

	std::vector<VkFence> _executeFence;

	std::vector<VkCommandBuffer> _cmdBuf;

	//Renderer map
	std::map<std::string, class VulkanRenderer*> _renderers;

#if IS_EDITOR
	//GUI pass(Editor)
	std::shared_ptr<class ImguiPassEditor> _imguiPassEditor;
#endif

};