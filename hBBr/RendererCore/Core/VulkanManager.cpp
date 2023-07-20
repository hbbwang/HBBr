#include "VulkanManager.h"
#include <vector>
#include "pugixml.hpp"

//导入Vulkan静态库
#pragma comment(lib ,"vulkan-1.lib")

VulkanManager::VulkanManager()
{
#if defined(_WIN32)
	_currentPlatform = EPlatform::Windows;
#elif defined(__ANDROID__)
	_currentPlatform = EPlatform::Android;
#elif defined(__linux__)
	_currentPlatform = EPlatform::Linux;
#endif

}

VulkanManager::~VulkanManager()
{
	if (_instance != VK_NULL_HANDLE)
		vkDestroyInstance(_instance, nullptr);
}

void VulkanManager::InitVulkan()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "hBBr";
	appInfo.pEngineName = "hBBr Engine" ;
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	//extensions
	std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME } ;
	#if defined(_WIN32)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#elif defined(__ANDROID__)
		extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	#elif defined(__linux__)
		extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	#endif
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		MessageOut(
			"Cannot find a compatible Vulkan installable client "
			"driver (ICD). Please make sure your driver supports "
			"Vulkan before continuing. The call to vkCreateInstance failed." , true);
	}
	else if (result != VK_SUCCESS) {
		MessageOut(
			"The call to vkCreateInstance failed. Please make sure "
			"you have a Vulkan installable client driver (ICD) before "
			"continuing.", true);
	}
}

void VulkanManager::InitInstance()
{
}

void VulkanManager::InitDevice()
{
}

void VulkanManager::InitDebug()
{
}

void VulkanManager::SetSurface(HWND hWnd)
{
}
