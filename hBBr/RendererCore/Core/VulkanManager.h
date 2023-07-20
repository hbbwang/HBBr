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
#elif defined(__ANDROID__)
	#include <vulkan/vulkan_android.h>
#elif defined(__linux__)
	#include <vulkan/vulkan_xcb.h>
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
	void DestroySurface(VkSurfaceKHR& surface);

	/* 获取Surface的大小 */
	VkExtent2D GetSurfaceSize(VkSurfaceKHR& surface);

	/* 检查Surface支持 */
	void CheckSurfaceSupport(VkSurfaceKHR& surface, VkSurfaceFormatKHR& surfaceFormat);

	/* 创建Swapchain */
	void CreateSwapchain(VkSurfaceKHR& surface, VkSurfaceFormatKHR& surfaceFormat, VkSwapchainKHR& newSwapchain);

	/* 释放Swapchain */
	void DestroySwapchain(VkSwapchainKHR& swapchain);

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

void MessageOut(const char* msg, bool bExit = false);