#pragma once
//Vulkan底层核心管理类
#include "../Common/Common.h"
#include <vulkan/vulkan.h>
#include "HString.h"
//Windows 
#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
	#include <stdlib.h>
	#include <Windows.h>
	#include <vulkan/vulkan_win32.h>
	#define DE_ASSERT(exp_, msgw_) _ASSERT_EXPR((exp_), HString(HString("\r\nMessage: ")+ HString(msgw_)).c_wstr());
#elif defined(__ANDROID__)
	#include <vulkan/vulkan_android.h>
#elif defined(__linux__)
	#include <vulkan/vulkan_xcb.h>
#endif

#ifndef DE_ASSERT
	#define DE_ASSERT ;
#endif

enum class EPlatform :uint8_t
{
	Windows		= 0,
	Linux		= 1,
	Android		= 2,
};

class VulkanManager
{
public:
	VulkanManager();
	~VulkanManager();

	/* 初始化Vulkan */

	void InitInstance(bool bEnableDebug);

	void InitDevice(VkSurfaceKHR surface = VK_NULL_HANDLE);

	void InitDebug();

	/* 创建Surface */
	void CreateSurface(void* hWnd, VkSurfaceKHR& newSurface);

	/* 释放Surface */
	void DestroySurface(VkSurfaceKHR surface);

	/* 获取Surface的大小 */
	VkExtent2D GetSurfaceSize(VkSurfaceKHR surface);

	/* 检查Surface支持 */
	void CheckSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR& surfaceFormat);

	/* 创建Swapchain */
	VkExtent2D CreateSwapchain(VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSwapchainKHR& newSwapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews);

	/* 释放Swapchain */
	void DestroySwapchain(VkSwapchainKHR swapchain, std::vector<VkImage> swapchainImages, std::vector<VkImageView> swapchainImageViews);

	/* 创建Vulkan image ,但是不带 mipmaps */
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, VkImage& image);

	/* 根据VkImageView ,创建Vulkan image view*/
	void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,  VkImageView& imageView);

	/* 创建Vulkan image view memory*/
	void CreateImageMemory(VkImage image, VkDeviceMemory& imageViewMemory, VkMemoryPropertyFlags memoryPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	void DestroyImage(VkImage& inImage);

	void DestroyImageMemory(VkDeviceMemory& imageViewMemory);

	void DestroyImageView(VkImageView& imageView);

	/* 创建Frame buffer */
	void CreateFrameBuffers(VkRenderPass renderPass, VkExtent2D FrameBufferSize,std::vector<std::vector<VkImageView>> imageViews, std::vector<VkFramebuffer>&frameBuffers);

	void DestroyFrameBuffers(std::vector<VkFramebuffer>frameBuffers);

	void CreateCommandPool();

	void DestroyCommandPool();

	void CreateCommandPool(VkCommandPool& commandPool);

	void DestroyCommandPool(VkCommandPool commandPool);

	void CreateCommandBuffer(VkCommandPool commandPool , VkCommandBuffer& cmdBuf);

	void DestroyCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer> cmdBufs);

	void BeginCommandBuffer(VkCommandBuffer cmdBuf , VkCommandBufferUsageFlags flag = 0);

	void EndCommandBuffer(VkCommandBuffer cmdBuf);

	void GetNextSwapchainIndex(VkSwapchainKHR swapchain, uint32_t& swapchainIndex);

	void Present(VkSwapchainKHR swapchain , uint32_t& swapchainIndex);

	/* Image 布局转换 */
	void Transition(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspects, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1);


	/* 获取平台 */
	__forceinline EPlatform GetPlatform()const {
		return _currentPlatform;
	}

	__forceinline VkInstance GetInstance()const {
		return _instance;
	}

	__forceinline uint32_t GetSwapchainBufferCount()const {
		return _swapchainBufferCount;
	}

	__forceinline VkDevice GetDevice()const {
		return _device;
	}

	__forceinline VkCommandPool GetCommandPool()const {
		return _commandPool;
	}

private:

	bool _bDebugEnable;

	EPlatform _currentPlatform;

	/* Get Memory Type Index */
	uint32_t FindMemoryTypeIndex(const VkMemoryRequirements* memory_requirements, const VkMemoryPropertyFlags required_properties);

	/* Get Suitable GPU Device */
	bool IsGPUDeviceSuitable(VkPhysicalDevice device);

	HString GetVkResult(VkResult code);

//Vulkan var
	
	VkInstance _instance;

	VkDevice _device;

	VkPhysicalDevice _gpuDevice;

	VkDebugReportCallbackCreateInfoEXT	debugCallbackCreateInfo{};

	VkDebugReportCallbackEXT			_debugReport;

	VkPhysicalDeviceProperties2			_gpuProperties{};

	VkPhysicalDeviceMemoryProperties	_gpuMemoryProperties{};

	VkPhysicalDeviceFeatures			_gpuFeatures{};

	VkPhysicalDeviceVulkan12Features	_gpuVk12Features{};

	VkQueue	_graphicsQueue;

	VkCommandPool _commandPool;

	int _graphicsQueueFamilyIndex;

	uint32_t _swapchainBufferCount;

	bool _enable_VK_KHR_display;

	//Debug marker
	static bool debugMarkerActive;
	static bool extensionPresent;
	static PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag;
	static PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName;
	static PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin;
	static PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd;
	static PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert;

};

void MessageOut(const char* msg, bool bExit = false, bool bMessageBox = false);