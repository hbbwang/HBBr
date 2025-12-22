#pragma once
#include <thread>
#include <memory>
#include "VulkanManager.h"

class VulkanSwapchain
{
	friend class VulkanApp;
	friend class VkWindow;
public:
	VulkanSwapchain(SDL_Window* window);
	~VulkanSwapchain();
protected:
	//Main Thread Objects
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	VkSurfaceFormatKHR SurfaceFormat{};
	VkSurfaceCapabilitiesKHR SurfaceCapabilities{};
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	VkExtent2D SurfaceSize{};
	std::vector<VkImage> SwapchainImages;
	std::vector<VkImageView>SwapchainImageViews;
	uint32_t CurrentFrameIndex = 0;
	std::vector<VkFence> ExecuteFence;
	//Render Thread Objects
	std::vector<VkSemaphore> QueueSemaphore;
	std::vector<VkSemaphore> AcquireSemaphore;
	std::vector<VkCommandBuffer> CmdBuf;
	//Functions
	void Update_MainThread();
	void Update_RenderThread();
	bool bIsInitialized = false;
private:
	SDL_Window* WindowHandle = nullptr;
};
