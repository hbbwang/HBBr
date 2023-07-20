#pragma once
//Vulkan底层核心管理类
#include <vulkan/vulkan.h>

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
	void InitVulkan();

	/* 设置Surface */
#if _WIN32
	void SetSurface(HWND hWnd);
#endif

	/* 获取平台 */
	__forceinline EPlatform GetPlatform()const {
		return _currentPlatform;
	}

	__forceinline VkInstance GetInstance()const {
		return _instance;
	}

private:

	EPlatform _currentPlatform;

	void InitInstance();

	void InitDevice();

	void InitDebug();

//Vulkan var
	
	VkInstance _instance;

	VkDevice _device;

	VkPhysicalDevice _gpuDevice;

	VkSurfaceKHR _surface;

};

void MessageOut(const char* msg, bool bExit = false)
{
#if defined(_WIN32)
	MessageBoxA(NULL, msg, "error", MB_ICONERROR);
#else
	printf(msg);
	fflush(stdout);
#endif
	if (bExit)
		exit(EXIT_FAILURE);
}
