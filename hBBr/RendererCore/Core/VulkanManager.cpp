#include "VulkanManager.h"
#include <vector>
#include <array>
#include "pugixml.hpp"
#include "HString.h"
#include "ConsoleDebug.h"
#include "RendererConfig.h"
#include "Texture.h"
#include "FileSystem.h"
#include "Shader.h"
//导入Vulkan静态库
#pragma comment(lib ,"vulkan-1.lib")
using namespace std;

std::unique_ptr<VulkanManager> VulkanManager::_vulkanManager;

PFN_vkCreateDebugReportCallbackEXT  fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT  fvkDestroyDebugReportCallbackEXT = nullptr;

bool VulkanManager::debugMarkerActive = false;
bool VulkanManager::extensionPresent = false;
PFN_vkDebugMarkerSetObjectTagEXT VulkanManager::vkDebugMarkerSetObjectTag = VK_NULL_HANDLE;
PFN_vkDebugMarkerSetObjectNameEXT VulkanManager::vkDebugMarkerSetObjectName = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerBeginEXT VulkanManager::vkCmdDebugMarkerBegin = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerEndEXT VulkanManager::vkCmdDebugMarkerEnd = VK_NULL_HANDLE;
PFN_vkCmdDebugMarkerInsertEXT VulkanManager::vkCmdDebugMarkerInsert = VK_NULL_HANDLE;

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT	flags,
	VkDebugReportObjectTypeEXT	obj_type,
	uint64_t	src_obj,
	size_t	location,
	int32_t	msg_code,
	const char* layer_prefix,
	const char* msg,
	void* user_data
)
{
	HString title;
	HString color;
	bool bError = false;
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		title = "INFO: "; color = "255,255,255";
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		title = "WARNING: "; color = "255,255,0";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		title = "PERFORMANCE WARNING: "; color = "255,175,0";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		title = "ERROR: "; color = "255,0,0";
		bError = true;
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		title = "DEBUG: "; color = "255,255,255";
	}
	//if (flags & VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT) {
	//	ConsoleDebug::print_endl(DString(": "));
	//}
	ConsoleDebug::print_endl(title + HString("@[") + layer_prefix + "]", color);
	ConsoleDebug::print_endl(msg, color);
	if(bError)
		MessageOut(HString(title + HString("@[") + layer_prefix + "]\n" + msg).c_str(),false,true);
	return false;
}

VulkanManager::VulkanManager(bool bDebug)
{
#if defined(_WIN32)
	_currentPlatform = EPlatform::Windows;
#elif defined(__ANDROID__)
	_currentPlatform = EPlatform::Android;
#elif defined(__linux__)
	_currentPlatform = EPlatform::Linux;
#endif
	_bDebugEnable = false;
	_graphicsQueueFamilyIndex = -1;
	_swapchainBufferCount = 3;
	_enable_VK_KHR_display = false;
	//Init global vulkan  
	InitInstance(bDebug);
	InitDevice();
	InitDebug();
	CreateCommandPool();
	CreateDescripotrPool(_descriptorPool);
}

VulkanManager::~VulkanManager()
{
	DestroyDescriptorPool(_descriptorPool);
	DestroyCommandPool();
	if (_bDebugEnable)
		fvkDestroyDebugReportCallbackEXT(_instance, _debugReport, nullptr);
	if (_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(_device, nullptr);
		_device = VK_NULL_HANDLE;
	}
	if (_instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(_instance, nullptr);
		_instance = VK_NULL_HANDLE;
	}
}

void VulkanManager::InitInstance(bool bEnableDebug)
{
	_bDebugEnable = bEnableDebug;
	//layers & extensions
	std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	std::vector<const char*> layers;

	//列举支持的layers和extensions
	{
		uint32_t count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		std::vector<VkLayerProperties> availableLaters(count);
		vkEnumerateInstanceLayerProperties(&count, availableLaters.data());
		ConsoleDebug::print_endl("----------Instance Layer Properties---------");
		for (uint32_t i = 0; i < count; i++)
		{
			char layerName[256];
			memcpy(layerName, availableLaters[i].layerName, 256);
			//_instance_layers.push_back(layerName);
			ConsoleDebug::print_endl(availableLaters[i].layerName + HString("  |  ") + availableLaters[i].description, "150,150,150");

			uint32_t ecount = 0;
			vkEnumerateInstanceExtensionProperties(availableLaters[i].layerName, &ecount, nullptr);
			std::vector<VkExtensionProperties> availableExts(ecount);
			vkEnumerateInstanceExtensionProperties(availableLaters[i].layerName, &ecount, availableExts.data());
			ConsoleDebug::print_endl("\tInstance Extension Properties---------");
			for (uint32_t i = 0; i < ecount; i++)
			{
				char extName[256];
				memcpy(extName, availableExts[i].extensionName, 256);
				ConsoleDebug::print_endl(HString("\t") + extName, "150,150,150");
				if (strcmp(availableExts[i].extensionName, VK_KHR_DISPLAY_EXTENSION_NAME) == 0)
				{
					layers.push_back(availableLaters[i].layerName);
					extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
					_enable_VK_KHR_display = true;
				}
			}
			ConsoleDebug::print_endl("\t------------------");
		}
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "hBBr";
	appInfo.pEngineName = "hBBr Engine";
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;

#if defined(_WIN32)
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
	extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
	if (_bDebugEnable)
	{
		layers.push_back("VK_LAYER_KHRONOS_validation");
		//RenderDoc支持
		//layers.push_back("VK_LAYER_RENDERDOC_Capture");//
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
		debugCallbackCreateInfo.flags =
			VK_DEBUG_REPORT_WARNING_BIT_EXT
			| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
			| VK_DEBUG_REPORT_ERROR_BIT_EXT
			;
		createInfo.pNext = &debugCallbackCreateInfo;
	}
	createInfo.enabledExtensionCount = (uint32_t)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = (uint32_t)layers.size();
	createInfo.ppEnabledLayerNames = layers.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		MessageOut( RendererLauguage::GetText("A000000").c_str() , true, true);
	}
	else if (result != VK_SUCCESS) {
		MessageOut(RendererLauguage::GetText("A000001").c_str(), true, true);
	}
}

void VulkanManager::InitDevice()
{
	//--------------Get GPU
	{
		uint32_t devicesCount = 0;
		vkEnumeratePhysicalDevices(_instance, &devicesCount, nullptr);
		std::vector<VkPhysicalDevice> gpu_list(devicesCount);
		vkEnumeratePhysicalDevices(_instance, &devicesCount, gpu_list.data());
		ConsoleDebug::print_endl("-----------GPU List-----------", "0,255,0");
		ConsoleDebug::print_endl("Found " + HString::FromUInt(devicesCount) + " GPU(s)", "222,255,255");
		_gpuProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		for (uint32_t i = 0; i < devicesCount; i++)
		{
			vkGetPhysicalDeviceProperties2(gpu_list[i], &_gpuProperties);
			ConsoleDebug::print_endl("Device Name:" + HString(_gpuProperties.properties.deviceName), "200,255,255");
			ConsoleDebug::print_endl("Device ID:" + HString::FromUInt(_gpuProperties.properties.deviceID), "150,150,150");

			if (IsGPUDeviceSuitable(gpu_list[i]))
			{
				_gpuDevice = gpu_list[i];
				break;
			}
			//
		}
		ConsoleDebug::print_endl("---------------------------", "0,255,0");
		if (_gpuDevice == VK_NULL_HANDLE)
		{
			MessageOut(RendererLauguage::GetText("A000002").c_str(),true, true);
		}
		vkGetPhysicalDeviceProperties2(_gpuDevice, &_gpuProperties);
		vkGetPhysicalDeviceMemoryProperties(_gpuDevice, &_gpuMemoryProperties);
	}
	//------------------Queue Family
	VkQueueFamilyProperties graphicsQueueFamilyProperty{};
	VkQueueFamilyProperties transferQueueFamilyProperty{};
	{
		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpuDevice, &family_count, nullptr);
		std::vector<VkQueueFamilyProperties> family_property_list(family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(_gpuDevice, &family_count, family_property_list.data());
		bool bFound_Graphics = false;
		//bool bFound_Transfer = false;
		for (uint32_t i = 0; i < family_count; i++) {
			//一般情况下Graphics queue同时支持多种功能,不需要另外获取Transfer等不同queue
			//但是据听说有的Graphics queue并不支持Present,所以需要同时判断Present支持
			if (bFound_Graphics == false && family_property_list[i].queueCount > 0 && (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT || family_property_list[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
			{
				bFound_Graphics = true;
				_graphicsQueueFamilyIndex = i;
				graphicsQueueFamilyProperty = family_property_list[i];
				//if (surface != VK_NULL_HANDLE)
				//{
				//	VkBool32 presentSupport = false;
				//	vkGetPhysicalDeviceSurfaceSupportKHR(_gpuDevice, i, surface, &presentSupport);
				//	if (presentSupport) {
				//		break;
				//	}
				//}
				break;
			}
			//if (bFound_Transfer == false && family_property_list[i].queueCount > 0 && family_property_list[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			//{
			//	bFound_Transfer = true;
			//	_transferQueueFamilyIndex = i;
			//	transferQueueFamilyProperty = family_property_list[i];
			//	break;
			//}
		}
		//if (!bFound_Graphics && !bFound_Transfer)
		if (!bFound_Graphics)
		{
			MessageOut(RendererLauguage::GetText("A000003").c_str(),true, true);
		}
	}
	std::vector <const char*> layers;
	std::vector <const char*> extensions;
	//列举Device支持的Layers和Extensions
	{
		uint32_t count = 0;
		vkEnumerateDeviceLayerProperties(_gpuDevice, &count, nullptr);
		std::vector<VkLayerProperties> availableLaters(count);
		vkEnumerateDeviceLayerProperties(_gpuDevice, &count, availableLaters.data());
		ConsoleDebug::print_endl("----------Device Layer Properties---------");
		for (uint32_t i = 0; i < count; i++) {
			char layerName[256];
			memcpy(layerName, availableLaters[i].layerName, 256);
			//_device_layers.push_back(layerName);
			ConsoleDebug::print_endl(availableLaters[i].layerName + HString("  |  ") + availableLaters[i].description, "150,150,150");
			//
			uint32_t ecount = 0;
			vkEnumerateDeviceExtensionProperties(_gpuDevice, availableLaters[i].layerName, &ecount, nullptr);
			std::vector<VkExtensionProperties> availableExts(ecount);
			vkEnumerateDeviceExtensionProperties(_gpuDevice, availableLaters[i].layerName, &ecount, availableExts.data());
			ConsoleDebug::print_endl("\tInstance Extension Properties---------");
			for (uint32_t i = 0; i < ecount; i++)
			{
				char extName[256];
				memcpy(extName, availableExts[i].extensionName, 256);
				ConsoleDebug::print_endl(HString("\t") + extName, "150,150,150");
				//Debug Marker
				{
					if (strcmp(availableExts[i].extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
					{
						extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
						debugMarkerActive = true;
						break;
					}
				}

			}
			ConsoleDebug::print_endl("\t------------------");
		}
	}
	extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	extensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
	//允许深度/模板图像的图像存储屏障仅设置了深度或模板位之一，而不是两者都设置。
	extensions.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
	if (_bDebugEnable)
	{
		layers.push_back("VK_LAYER_KHRONOS_validation");
		extensions.push_back("VK_EXT_debug_marker");
		extensions.push_back("VK_EXT_tooling_info");
	}
	//---------------------Create Device
	std::array<std::vector<float>, 1> prior;
	prior[0].resize(graphicsQueueFamilyProperty.queueCount, 0);
	//prior[1].resize(transferQueueFamilyProperty.queueCount, 0);

	std::vector<VkDeviceQueueCreateInfo> queue_create_info;
	VkDeviceQueueCreateInfo device_queue_create_info = {};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex = _graphicsQueueFamilyIndex;
	device_queue_create_info.queueCount = graphicsQueueFamilyProperty.queueCount;
	device_queue_create_info.pQueuePriorities = prior[0].data();
	queue_create_info.push_back(device_queue_create_info);
	//if (_graphicsQueueFamilyIndex != _transferQueueFamilyIndex)
	//{
	//	VkDeviceQueueCreateInfo device_transfer_queue_create_info = {};
	//	device_transfer_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//	device_transfer_queue_create_info.queueFamilyIndex = _transferQueueFamilyIndex;
	//	device_transfer_queue_create_info.queueCount = transferQueueFamilyProperty.queueCount;
	//	device_transfer_queue_create_info.pQueuePriorities = prior[1].data();
	//	queue_create_info.push_back(device_transfer_queue_create_info);
	//}
	vkGetPhysicalDeviceFeatures(_gpuDevice, &_gpuFeatures);
	//开启vk的gpu特殊功能
	_gpuVk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	_gpuVk12Features.separateDepthStencilLayouts = VK_TRUE;

	VkDeviceCreateInfo device_create_info = {};
	memset(&device_create_info, 0, sizeof(VkDeviceCreateInfo));
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_info.size());
	device_create_info.pQueueCreateInfos = queue_create_info.data();
	device_create_info.ppEnabledLayerNames = layers.data();
	device_create_info.enabledLayerCount = (uint32_t)layers.size();
	device_create_info.ppEnabledExtensionNames = extensions.data();
	device_create_info.enabledExtensionCount = (uint32_t)extensions.size();
	device_create_info.pEnabledFeatures = &_gpuFeatures;
	device_create_info.pNext = &_gpuVk12Features;
	auto result = vkCreateDevice(_gpuDevice, &device_create_info, nullptr, &_device);
	if(result!= VK_SUCCESS) 
		MessageOut((RendererLauguage::GetText("A000004") + GetVkResult(result)).c_str() , true, true);
	vkGetDeviceQueue(_device, _graphicsQueueFamilyIndex, 0, &_graphicsQueue);
	//vkGetDeviceQueue(_device, _transferQueueFamilyIndex, 0, &_transfer_Queue);
	if (_enable_VK_KHR_display)
	{
		uint32_t displayCount = 0;
		vkGetPhysicalDeviceDisplayPropertiesKHR(_gpuDevice, &displayCount, nullptr);
		std::vector<VkDisplayPropertiesKHR> displayPro(displayCount);
		vkGetPhysicalDeviceDisplayPropertiesKHR(_gpuDevice, &displayCount, displayPro.data());
		ConsoleDebug::print_endl("-----------Display info-----------", "0,255,0");
		for (uint32_t i = 0; i < displayCount; i++)
		{
			ConsoleDebug::print_endl(HString("-----------Display ") + HString::FromInt(i) + " -----------", "200,255,255");
			ConsoleDebug::print_endl("Display Name:" + HString(displayPro[i].displayName), "200,255,255");
			ConsoleDebug::print_endl("Display Width:" + HString::FromInt(displayPro[i].physicalResolution.width), "200,255,255");
			ConsoleDebug::print_endl("Display Height:" + HString::FromInt(displayPro[i].physicalResolution.height), "200,255,255");
			uint32_t count = 0;
			vkGetDisplayModePropertiesKHR(_gpuDevice, displayPro[i].display, &count, nullptr);
			std::vector<VkDisplayModePropertiesKHR> displayMode(count);
			vkGetDisplayModePropertiesKHR(_gpuDevice, displayPro[i].display, &count, displayMode.data());
			for (uint32_t i = 0; i < count; i++)
			{
				ConsoleDebug::print_endl("Display Refresh Rate:" + HString::FromInt(displayMode[i].parameters.refreshRate), "200,255,255");
			}
		}
	}
	if (debugMarkerActive)
	{
		//Debug Marker
		vkDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(_device, "vkDebugMarkerSetObjectTagEXT");
		vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(_device, "vkDebugMarkerSetObjectNameEXT");
		vkCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(_device, "vkCmdDebugMarkerBeginEXT");
		vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(_device, "vkCmdDebugMarkerEndEXT");
		vkCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(_device, "vkCmdDebugMarkerInsertEXT");
	}
}

void VulkanManager::InitDebug()
{
	if (_bDebugEnable)
	{
		fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
		fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
		if (nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestroyDebugReportCallbackEXT)
		{
			MessageOut("Vulkan ERROR: Cant fetch debug function pointers.");
		}
		fvkCreateDebugReportCallbackEXT(_instance, &debugCallbackCreateInfo, nullptr, &_debugReport);
	}
}

uint32_t VulkanManager::FindMemoryTypeIndex(const VkMemoryRequirements* memory_requirements, const VkMemoryPropertyFlags required_properties)
{
	for (uint32_t i = 0; i < _gpuMemoryProperties.memoryTypeCount; ++i) {
		if (memory_requirements->memoryTypeBits & (1 << i)) {
			if ((_gpuMemoryProperties.memoryTypes[i].propertyFlags & required_properties) == required_properties) {
				return i;
			}
		}
	}
	MessageOut("Cound not find memory type.");
	return UINT32_MAX;
}

bool VulkanManager::IsGPUDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		deviceFeatures.geometryShader;
}

void VulkanManager::CreateSurface(void* hWnd , VkSurfaceKHR& newSurface)
{
#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR info={};
	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.hwnd = (HWND)hWnd;
	info.hinstance = GetModuleHandle(NULL);
	auto result = vkCreateWin32SurfaceKHR(_instance, &info, nullptr, &newSurface);
	if (result != VK_SUCCESS)
	{
		MessageOut(RendererLauguage::GetText("A000005").c_str(),true, true);
	}
#endif
}

void VulkanManager::DestroySurface(VkSurfaceKHR surface)
{
	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(_instance, surface, nullptr);
	}
}

VkExtent2D VulkanManager::GetSurfaceSize(VkSurfaceKHR surface)
{
	const int SwapchainBufferCount = 3;
	VkExtent2D surfaceSize = {};
	VkSurfaceCapabilitiesKHR _surfaceCapabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpuDevice, surface, &_surfaceCapabilities);
	if (_surfaceCapabilities.currentExtent.width < UINT32_MAX && _surfaceCapabilities.currentExtent.width>0) {
		surfaceSize.width = _surfaceCapabilities.currentExtent.width;
		surfaceSize.height = _surfaceCapabilities.currentExtent.height;
	}
	else {
		surfaceSize.width = _surfaceCapabilities.maxImageExtent.width < (uint32_t)surfaceSize.width ? _surfaceCapabilities.maxImageExtent.width : (uint32_t)surfaceSize.width;
		surfaceSize.width = _surfaceCapabilities.minImageExtent.width > (uint32_t)surfaceSize.width ? _surfaceCapabilities.minImageExtent.width : (uint32_t)surfaceSize.width;
		surfaceSize.height = _surfaceCapabilities.maxImageExtent.height < (uint32_t)surfaceSize.height ? _surfaceCapabilities.maxImageExtent.height : (uint32_t)surfaceSize.height;
		surfaceSize.height = _surfaceCapabilities.minImageExtent.height > (uint32_t)surfaceSize.height ? _surfaceCapabilities.minImageExtent.height : (uint32_t)surfaceSize.height;
	}
	_swapchainBufferCount = _surfaceCapabilities.minImageCount > SwapchainBufferCount ? _surfaceCapabilities.minImageCount : SwapchainBufferCount;
	_swapchainBufferCount = _surfaceCapabilities.maxImageCount < _swapchainBufferCount ? _surfaceCapabilities.maxImageCount : _swapchainBufferCount;
	return surfaceSize;
}

void VulkanManager::CheckSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR& surfaceFormat)
{
	//Check Support
	VkBool32 IsSupportSurface = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(_gpuDevice, _graphicsQueueFamilyIndex, surface, &IsSupportSurface);
	if (!IsSupportSurface)
	{
		MessageOut(RendererLauguage::GetText("A000006").c_str(),true, true);
	}
	{
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM,VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		uint32_t avail_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpuDevice, surface, &avail_count, NULL);
		std::vector<VkSurfaceFormatKHR> avail_format;
		avail_format.resize((int)avail_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpuDevice, surface, &avail_count, avail_format.data());
		if (avail_count == 1)
		{
			if (avail_format[0].format == VK_FORMAT_UNDEFINED)
			{
				surfaceFormat.format = requestSurfaceImageFormat[0];
				surfaceFormat.colorSpace = requestSurfaceColorSpace;
			}
			else
			{
				// No point in searching another format
				surfaceFormat = avail_format[0];
			}
		}
		else
		{
			// Request several formats, the first found will be used
			for (int request_i = 0; request_i < _countof(requestSurfaceImageFormat); request_i++)
				for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
					if (avail_format[avail_i].format == requestSurfaceImageFormat[request_i] && avail_format[avail_i].colorSpace == requestSurfaceColorSpace)
					{
						surfaceFormat = avail_format[avail_i];
						return;
					}
			// If none of the requested image formats could be found, use the first available
			surfaceFormat = avail_format[0];
		}
	}
}

VkExtent2D VulkanManager::CreateSwapchain(VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat , VkSwapchainKHR &newSwapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews)
{
	//ConsoleDebug::print_endl("Create Swapchain KHR.");
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	//if (_winInfo.vsync)
	{
		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, nullptr);
		std::vector<VkPresentModeKHR> presentModes(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, presentModes.data());
		for (int i = 0; i < presentModes.size(); i++)
		{
			//if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			//{
			//	present_mode = presentModes[i];//这个垂直同步在Nvidia有bug ?
			//	//break;
			//}
			if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
			{
				present_mode = presentModes[i];//垂直同步
				break;
			}
		}
	}

	//Get surface size 
	VkExtent2D _surfaceSize = GetSurfaceSize(surface);
	if (_surfaceSize.width <= 0 && _surfaceSize.height <= 0)
	{
		return _surfaceSize;
	}
	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.minImageCount = _swapchainBufferCount;
	info.imageFormat = surfaceFormat.format;
	info.imageColorSpace = surfaceFormat.colorSpace;
	info.imageExtent = _surfaceSize;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT	//支持在RenderPass中作为color附件，并且在subpass中进行传递
		| VK_IMAGE_USAGE_TRANSFER_SRC_BIT					//支持复制到其他图像
		;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//是否半透明，用于组合其他表面,这里我们不需要
	info.presentMode = present_mode;
	info.clipped = VK_TRUE;//是否不渲染看不见的位置
	//替补swapchain,在重置这个swapchain的时候会比较有用，但是事实上我们可以用更暴力的方法重置swapchain，也就是完全重写
	info.oldSwapchain = VK_NULL_HANDLE;
	auto result = vkCreateSwapchainKHR(_device, &info, nullptr, &newSwapchain);
	if (result != VK_SUCCESS)
	{
		MessageOut((RendererLauguage::GetText("A000007").c_str() + GetVkResult(result)).c_str(), true, true);
	}
	vkGetSwapchainImagesKHR(_device, newSwapchain, &_swapchainBufferCount, nullptr);
	swapchainImages.resize(_swapchainBufferCount);
	swapchainImageViews.resize(_swapchainBufferCount);
	vkGetSwapchainImagesKHR(_device, newSwapchain, &_swapchainBufferCount, swapchainImages.data());
	//创建ImageView
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		CreateImageView(swapchainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, swapchainImageViews[i]);
	}
	//Swapchain转换到呈现模式
	VkCommandBuffer buf;
	AllocateCommandBuffer(_commandPool, buf);
	BeginCommandBuffer(buf, 0);
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		Transition(buf, swapchainImages[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
	EndCommandBuffer(buf);
	SubmitQueueImmediate({buf});
	vkDeviceWaitIdle(GetDevice());
	FreeCommandBuffers(_commandPool, { buf });
	return _surfaceSize;
}

VkExtent2D VulkanManager::CreateSwapchain(VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSwapchainKHR& newSwapchain, std::vector<std::shared_ptr<class Texture>>& textures)
{
	//ConsoleDebug::print_endl("Create Swapchain KHR.");
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	//if (_winInfo.vsync)
	{
		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, nullptr);
		std::vector<VkPresentModeKHR> presentModes(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, presentModes.data());
		for (int i = 0; i < presentModes.size(); i++)
		{
			//if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			//{
			//	present_mode = presentModes[i];//这个垂直同步在Nvidia有bug ?
			//	//break;
			//}
			if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
			{
				present_mode = presentModes[i];//垂直同步
				break;
			}
		}
	}

	//Get surface size 
	VkExtent2D _surfaceSize = GetSurfaceSize(surface);
	if (_surfaceSize.width <= 0 && _surfaceSize.height <= 0)
	{
		return _surfaceSize;
	}
	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.minImageCount = _swapchainBufferCount;
	info.imageFormat = surfaceFormat.format;
	info.imageColorSpace = surfaceFormat.colorSpace;
	info.imageExtent = _surfaceSize;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT	//支持在RenderPass中作为color附件，并且在subpass中进行传递
		| VK_IMAGE_USAGE_TRANSFER_SRC_BIT					//支持复制到其他图像
		;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//是否半透明，用于组合其他表面,这里我们不需要
	info.presentMode = present_mode;
	info.clipped = VK_TRUE;//是否不渲染看不见的位置
	//替补swapchain,在重置这个swapchain的时候会比较有用，但是事实上我们可以用更暴力的方法重置swapchain，也就是完全重写
	info.oldSwapchain = VK_NULL_HANDLE;
	auto result = vkCreateSwapchainKHR(_device, &info, nullptr, &newSwapchain);
	if (result != VK_SUCCESS)
	{
		MessageOut((RendererLauguage::GetText("A000007").c_str() + GetVkResult(result)).c_str(), true, true);
	}
	vkGetSwapchainImagesKHR(_device, newSwapchain, &_swapchainBufferCount, nullptr);
	textures.resize(_swapchainBufferCount);
	std::vector<VkImage> images(_swapchainBufferCount);
	vkGetSwapchainImagesKHR(_device, newSwapchain, &_swapchainBufferCount, images.data());
	//创建ImageView
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		textures[i].reset(new Texture(true));
		textures[i]->_image = images[i];
		textures[i]->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		textures[i]->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		textures[i]->_format = surfaceFormat.format;
		textures[i]->_usageFlags = info.imageUsage;
		textures[i]->_textureName = "Swapchain Image " + HString::FromInt(i);
		CreateImageView(textures[i]->_image, surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, textures[i]->_imageView );
	}
	//Swapchain转换到呈现模式
	VkCommandBuffer buf;
	AllocateCommandBuffer(_commandPool, buf);
	BeginCommandBuffer(buf, 0);
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		//Transition(buf, swapchainImages[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		textures[i]->Transition(buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
	EndCommandBuffer(buf);
	SubmitQueueImmediate({ buf });
	vkDeviceWaitIdle(GetDevice());
	FreeCommandBuffers(_commandPool, { buf });
	return _surfaceSize;
}

void VulkanManager::DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews)
{
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		DestroyImageView(swapchainImageViews[i]);
	}
	swapchainImageViews.clear();
	swapchainImages.clear();
	//
	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
}

void VulkanManager::DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<std::shared_ptr<class Texture>>& textures)
{
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		DestroyImageView(textures[i]->_imageView);
		textures[i]->_image = VK_NULL_HANDLE;
		textures[i]->_imageView = VK_NULL_HANDLE;
	}
	textures.clear();
	//
	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
}

void VulkanManager::CreateImage(uint32_t width , uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, VkImage& image)
{
	VkExtent2D texSize = {};
	texSize.width = width;
	texSize.height = height;
	VkImageCreateInfo	create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.flags = 0;
	create_info.format = format;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.usage = usageFlags;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.arrayLayers = 1;
	create_info.mipLevels = 1;
	create_info.extent.depth = 1;
	create_info.extent.width = texSize.width;
	create_info.extent.height = texSize.height;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	create_info.pQueueFamilyIndices = nullptr;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	auto result = vkCreateImage(_device, &create_info, nullptr, &image);
	if (result != VK_SUCCESS)
	{
		MessageOut("Create vulkan image failed.",false,false);
	}
}

void VulkanManager::CreateImageView(VkImage inImage, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView)
{
	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.flags = 0;
	image_view_create_info.image = inImage;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = format;
	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.subresourceRange.aspectMask = aspectFlags;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.layerCount = 1;
	image_view_create_info.subresourceRange.levelCount = 1;
	vkCreateImageView(_device, &image_view_create_info, nullptr, &imageView);
}

void VulkanManager::CreateImageMemory(VkImage inImage, VkDeviceMemory& imageViewMemory, VkMemoryPropertyFlags memoryPropertyFlag)
{
	if (inImage == VK_NULL_HANDLE)
	{
		MessageOut("Create vulkan image view failed.VkImage is NULL.", false, false);
		return;
	}
	VkMemoryRequirements mem_requirement;
	vkGetImageMemoryRequirements(_device, inImage, &mem_requirement);
	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = mem_requirement.size;
	memory_allocate_info.memoryTypeIndex = FindMemoryTypeIndex(&mem_requirement, memoryPropertyFlag);
	auto err = vkAllocateMemory(_device, &memory_allocate_info, nullptr, &imageViewMemory);
	if (VK_SUCCESS != err) {
		MessageOut("Create vulkan image view failed.VkImage is NULL.", false, false);
	}
	err = vkBindImageMemory(_device, inImage, imageViewMemory, 0);
	if (VK_SUCCESS != err) {
		MessageOut("Create vulkan image view failed.VkImage is NULL.", false, false);
	}
}

void VulkanManager::Transition(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspects, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin , uint32_t mipLevelCount)
{
	VkImageMemoryBarrier imageBarrier = {};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.pNext = NULL;
	imageBarrier.oldLayout = oldLayout;
	imageBarrier.newLayout = newLayout;
	imageBarrier.image = image;
	imageBarrier.subresourceRange.aspectMask = aspects;
	imageBarrier.subresourceRange.baseMipLevel = mipLevelBegin;
	imageBarrier.subresourceRange.levelCount = mipLevelCount;
	imageBarrier.subresourceRange.layerCount = 1;
	VkPipelineStageFlags srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	switch (oldLayout) {
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imageBarrier.srcAccessMask =VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		srcFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		srcFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	}

	switch (newLayout) {
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageBarrier.dstAccessMask |=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dstFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;
	}
	vkCmdPipelineBarrier(cmdBuffer, srcFlags, dstFlags, 0, 0, NULL, 0, NULL, 1,
		&imageBarrier);
}

void VulkanManager::DestroyImage(VkImage& image)
{
	if (image != VK_NULL_HANDLE)
	{
		vkDestroyImage(_device, image, nullptr);
	}
}

void VulkanManager::DestroyImageMemory(VkDeviceMemory& imageViewMemory)
{
	if (imageViewMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(_device, imageViewMemory, nullptr);
	}
}

void VulkanManager::DestroyImageView(VkImageView& imageView)
{
	if (imageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(_device, imageView, nullptr);
	}
}

void VulkanManager::CreateFrameBuffers(VkRenderPass renderPass, VkExtent2D FrameBufferSize, std::vector<std::vector<VkImageView>> imageViews, std::vector<VkFramebuffer>& frameBuffers)
{
	frameBuffers.resize(_swapchainBufferCount);
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		std::vector<VkImageView> attachments;
		for (int v = 0; v < imageViews.size(); v++)
		{
			attachments.push_back(imageViews[v][i]);
		}
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = FrameBufferSize.width;
		framebufferInfo.height = FrameBufferSize.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VulkanManager::DestroyFrameBuffers(std::vector<VkFramebuffer>& frameBuffers)
{
	for (int i = 0; i < frameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(_device, frameBuffers[i], nullptr);
	}
	frameBuffers.clear();
}

void VulkanManager::CreateCommandPool()
{
	CreateCommandPool(_commandPool);
}

void VulkanManager::DestroyCommandPool()
{
	DestroyCommandPool(_commandPool);
}

void VulkanManager::CreateCommandPool(VkCommandPool& commandPool)
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = NULL;
	cmdPoolInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &commandPool);
}

void VulkanManager::DestroyCommandPool(VkCommandPool commandPool)
{
	if (commandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(_device, commandPool, nullptr);
	}
}

void VulkanManager::AllocateCommandBuffer(VkCommandPool commandPool, VkCommandBuffer& cmdBuf)
{
	VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
	cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocInfo.pNext = NULL;
	cmdBufAllocInfo.commandPool = commandPool;
	cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocInfo.commandBufferCount = 1;
	VkResult result = vkAllocateCommandBuffers(_device, &cmdBufAllocInfo,&cmdBuf);
	if (result != VK_SUCCESS)
	{
		MessageOut(RendererLauguage::GetText("A000008").c_str(), true, true);
	}
}

void VulkanManager::FreeCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer> cmdBufs)
{
	if (cmdBufs.size() > 0)
	{
		vkFreeCommandBuffers(_device, commandPool, (uint32_t)cmdBufs.size(), cmdBufs.data());
	}
}

void VulkanManager::BeginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferUsageFlags flag)
{
	//VkCommandBufferBeginInfo.flags:
	//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: 命令缓冲区将在执行一次后立即重新记录。
	//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 这是一个辅助缓冲区，它限制在在一个渲染通道中。
	//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 命令缓冲区也可以重新提交，同时它也在等待执行。
	VkCommandBufferBeginInfo info={};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = flag ;
	info.pInheritanceInfo = nullptr;//继承,不用
	vkBeginCommandBuffer(cmdBuf,&info);
}

void VulkanManager::EndCommandBuffer(VkCommandBuffer cmdBuf)
{
	vkEndCommandBuffer(cmdBuf);
}

void VulkanManager::BeginRenderPass(VkCommandBuffer cmdBuf, VkFramebuffer framebuffer, VkRenderPass renderPass, VkExtent2D areaSize, std::vector<VkAttachmentDescription>_attachmentDescs, std::array<float, 4> clearColor)
{
	VkRenderPassBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.framebuffer = framebuffer;
	info.renderPass = renderPass;
	info.renderArea.offset = { 0, 0 };
	info.renderArea.extent = areaSize;
	info.clearValueCount = (uint32_t)_attachmentDescs.size();
	std::vector < VkClearValue > clearValues;
	for (int i = 0; i < (int)_attachmentDescs.size(); i++)
	{
		VkClearValue clearValue = {};
		if (_attachmentDescs[i].finalLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL || _attachmentDescs[i].finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			clearValue.depthStencil.depth = 0.0f;
			clearValue.depthStencil.stencil = 1.0f;
		}
		else
		{
			clearValue.color.float32[0]= clearColor[0];
			clearValue.color.float32[1] = clearColor[1];
			clearValue.color.float32[2] = clearColor[2];
			clearValue.color.float32[3] = clearColor[3];
		}
		clearValues.push_back(clearValue);
	}

	info.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmdBuf, &info, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanManager::EndRenderPass(VkCommandBuffer cmdBuf)
{
	vkCmdEndRenderPass(cmdBuf);
}

void VulkanManager::GetNextSwapchainIndex(VkSwapchainKHR swapchain, VkSemaphore semaphore, uint32_t* swapchainIndex)
{
	VkResult result = vkAcquireNextImageKHR(_device, swapchain, UINT16_MAX, semaphore, VK_NULL_HANDLE, swapchainIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		MessageOut(RendererLauguage::GetText("A000009").c_str(), false, true);
	}
	else if (result != VK_SUCCESS)
	{
		MessageOut(RendererLauguage::GetText("A000010").c_str(), false, true);
	}
}

void VulkanManager::Present(VkSwapchainKHR swapchain, VkSemaphore semaphore, uint32_t& swapchainImageIndex)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &swapchainImageIndex;
	presentInfo.pResults = nullptr; // Optional
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphore;
	auto result = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		//MessageOut("VK_ERROR_OUT_OF_DATE_KHR.Swapchain need to reset.", false, false);
	}
	else if (result != VK_SUCCESS)
	{
		MessageOut(RendererLauguage::GetText("A000011").c_str(), false, true);
	}
}

void VulkanManager::CreatePipelineLayout(std::vector <VkDescriptorSetLayout> descriptorSetLayout, VkPipelineLayout& pipelineLayout)
{
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.flags = 0;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;
	info.pSetLayouts = descriptorSetLayout.data();
	info.setLayoutCount = (uint32_t)descriptorSetLayout.size();
	auto result = vkCreatePipelineLayout(_device, &info, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		MessageOut("vkCreatePipelineLayout error!",true,true);
	}
}

void VulkanManager::DestroyPipelineLayout(VkPipelineLayout& pipelineLayout)
{
	if (pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(_device, pipelineLayout, nullptr);
	}
	pipelineLayout = VK_NULL_HANDLE;
}

void VulkanManager::CreateDescripotrPool(VkDescriptorPool& pool)
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		//{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },	//这是一个image和sampler的组合descriptor
		//{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },			//纯image,不带sampler
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
		//{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		//{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		//{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3000 },
		//{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 80 }
	};
	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.pNext = NULL;
	info.maxSets = 10000;
	info.poolSizeCount = (uint32_t)std::size(pool_sizes);
	info .pPoolSizes = pool_sizes;
	vkCreateDescriptorPool(_device, &info, nullptr, &pool);
}

void VulkanManager::DestroyDescriptorPool(VkDescriptorPool& pool)
{
	if (pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(_device, pool, nullptr);	
	}
	pool = VK_NULL_HANDLE;
}

void VulkanManager::CreateDescripotrSetLayout(VkDescriptorType descriptorType, uint32_t bindingCount, VkDescriptorSetLayout& descriptorSetLayout, VkShaderStageFlags shaderStageFlags)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(bindingCount);
	for (uint32_t i = 0; i < bindingCount; i++)
	{
		bindings[i] = {};
		bindings[i].descriptorType = descriptorType;
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = shaderStageFlags;
		bindings[i].binding = i;
		bindings[i].pImmutableSamplers = nullptr;
	}
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = (uint32_t)bindings.size();
	info.pBindings = bindings.data();
	auto result = vkCreateDescriptorSetLayout(_device, &info, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		MessageOut("Create Descriptor Set Layout Error!!", false, true);
	}	
}

void VulkanManager::DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
	if (descriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(_device, descriptorSetLayout, nullptr);
	}
}

void VulkanManager::CreateFrameBuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, std::vector<VkImageView>attachments, VkFramebuffer& framebuffer)
{
	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.flags = 0;
	info.pNext = 0;
	info.layers = 1;
	info.renderPass = renderPass;
	info.width = width;
	info.height = height;
	info.attachmentCount = (uint32_t)attachments.size();
	info.pAttachments = attachments.data();
	auto result = vkCreateFramebuffer(_device, &info, nullptr, &framebuffer);
}

void VulkanManager::DestroyFrameBuffer(VkFramebuffer& framebuffer)
{
	if (framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(_device, framebuffer, nullptr);
		framebuffer = VK_NULL_HANDLE;
	}
}

void VulkanManager::AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, uint32_t newDescriptorSetCount , std::vector<VkDescriptorSet>&  descriptorSet)
{
	std::vector<VkDescriptorSetLayout> setLayout;
	setLayout.resize(newDescriptorSetCount);
	descriptorSet.resize(newDescriptorSetCount);
	for (int i = 0; i < (int)newDescriptorSetCount; i++)
	{
		setLayout[i] = descriptorSetLayout;
	}
	//
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = pool;
	info.descriptorSetCount = static_cast<uint32_t>(setLayout.size());
	info.pSetLayouts = setLayout.data();
	auto result = vkAllocateDescriptorSets(_device, &info, descriptorSet.data());
	if (result != VK_SUCCESS)
	{
		MessageOut("Vulkan ERROR: Allocate Descriptor Sets Failed.", false, true);
	}
}

void VulkanManager::FreeDescriptorSet(VkDescriptorPool pool, std::vector<VkDescriptorSet>& descriptorSet)
{
	if (descriptorSet.size() > 0)
	{
		vkFreeDescriptorSets(_device, pool, (uint32_t)descriptorSet.size(), descriptorSet.data());
	}
	descriptorSet.clear();
}

void VulkanManager::CreateSemaphore(VkSemaphore& semaphore)
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &semaphore);
}

void VulkanManager::DestroySemaphore(VkSemaphore semaphore)
{
	if (semaphore  != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(_device, semaphore, nullptr);
	}
	semaphore = VK_NULL_HANDLE;
}

void VulkanManager::CreateRenderSemaphores(std::vector<VkSemaphore>& semaphore)
{
	semaphore.resize(_swapchainBufferCount);
	for (int i = 0; i < semaphore.size(); i++)
	{
		CreateSemaphore(semaphore[i]);
	}
}

void VulkanManager::DestroyRenderSemaphores(std::vector<VkSemaphore>& semaphore)
{
	for (int i = 0; i < semaphore.size(); i++)
	{
		if (semaphore[i] != VK_NULL_HANDLE)
		{
			DestroySemaphore(semaphore[i]);
		}
	}
	semaphore.clear();
}

void VulkanManager::CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo& info, VkPipeline& pipeline)
{
	auto result = vkCreateGraphicsPipelines(_device, nullptr, 1, &info, nullptr, &pipeline);
	if (result != VK_SUCCESS)
	{
		MessageOut(RendererLauguage::GetText("A000012").c_str(), true, true);
	}
}

void VulkanManager::CreateRenderPass(std::vector<VkAttachmentDescription>attachmentDescs, std::vector<VkSubpassDependency>subpassDependencys, std::vector<VkSubpassDescription>subpassDescs, VkRenderPass& renderPass)
{
	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.flags = NULL;
	info.attachmentCount = (uint32_t)attachmentDescs.size();
	info.pAttachments = attachmentDescs.data();
	info.dependencyCount = (uint32_t)subpassDependencys.size();
	info.pDependencies = subpassDependencys.data();
	info.pNext = NULL;
	info.subpassCount = (uint32_t)subpassDescs.size();
	info.pSubpasses = subpassDescs.data();
	vkCreateRenderPass(_device, &info, nullptr, &renderPass);
}

void VulkanManager::DestroyRenderPass(VkRenderPass& renderPass)
{
	if (renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(_device, renderPass, nullptr);
	}
	renderPass = VK_NULL_HANDLE;
}

void VulkanManager::CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize bufferSize, VkBuffer& buffer)
{
	VkBufferCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = VK_NULL_HANDLE;
	create_info.usage = usage;
	create_info.size = bufferSize;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(_device, &create_info, nullptr, &buffer) != VK_SUCCESS) {
		MessageOut("[ Create Buffer ] Create Uniform Buffer Failed.", true, true, "255,2,0");
	}
}

void VulkanManager::DestroyBuffer(VkBuffer& buffer)
{
	if (buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(_device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}
}

void VulkanManager::CreateShaderModule(std::vector<char> data, VkShaderModule& shaderModule)
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = data.size();
	info.pCode = reinterpret_cast<const uint32_t*>(data.data());
	vkCreateShaderModule(_device, &info, nullptr, &shaderModule);
}

void VulkanManager::CreateShaderModule(VkDevice device, std::vector<char> data, VkShaderModule& shaderModule)
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = data.size();
	info.pCode = reinterpret_cast<const uint32_t*>(data.data());
	vkCreateShaderModule(device, &info, nullptr, &shaderModule);
}

void VulkanManager::AllocateBufferMemory(VkBuffer buffer, VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags propertyFlags)
{
	VkMemoryRequirements mem_requirement;
	vkGetBufferMemoryRequirements(_device, buffer, &mem_requirement);
	VkMemoryAllocateInfo mem_allocate_info{};
	mem_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_allocate_info.allocationSize = mem_requirement.size;
	mem_allocate_info.memoryTypeIndex = FindMemoryTypeIndex(&mem_requirement,propertyFlags);
	if (vkAllocateMemory(_device, &mem_allocate_info, nullptr, &bufferMemory) != VK_SUCCESS) {
		MessageOut("[ Create Buffer ] Allocate Memory Failed! ", true, true, "255,2,0");
	}
	if (vkBindBufferMemory(_device, buffer, bufferMemory, 0) != VK_SUCCESS) {
		MessageOut("[ Create Buffer ] Bind Buffer Memory!  ", true, true, "255,2,0");
	}
}

void VulkanManager::FreeBufferMemory(VkDeviceMemory& bufferMemory)
{
	if (bufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(_device, bufferMemory, nullptr);
	}
}

void VulkanManager::SubmitQueueImmediate(std::vector<VkCommandBuffer> cmdBufs, VkPipelineStageFlags waitStageMask, VkQueue queue)
{
	if (cmdBufs.size() <= 0)
	{
		MessageOut(RendererLauguage::GetText("A000013").c_str(), true, true);
	}
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pWaitDstStageMask = &waitStageMask;
	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = NULL;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores = NULL;
	info.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
	info.pCommandBuffers = cmdBufs.data();
	VkResult result;
	if (queue == VK_NULL_HANDLE)
		result = vkQueueSubmit(_graphicsQueue, 1, &info, VK_NULL_HANDLE);
	else
		result = vkQueueSubmit(queue, 1, &info, VK_NULL_HANDLE);
	if(result != VK_SUCCESS)
		MessageOut(" [Submit Queue Immediate]vkQueueSubmit error", false, false);
	vkQueueWaitIdle(_graphicsQueue);
}

void VulkanManager::SubmitQueue(std::vector<VkCommandBuffer> cmdBufs, std::vector <VkSemaphore> lastSemaphore, std::vector <VkSemaphore> newSemaphore, VkPipelineStageFlags waitStageMask, VkQueue queue)
{
	if (cmdBufs.size() <= 0)
	{
		MessageOut(RendererLauguage::GetText("A000013").c_str(), true, true);
	}
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pWaitDstStageMask = &waitStageMask;
	info.waitSemaphoreCount = (uint32_t)lastSemaphore.size();
	info.pWaitSemaphores = lastSemaphore.data();
	info.signalSemaphoreCount = (uint32_t)newSemaphore.size();
	info.pSignalSemaphores = newSemaphore.data();
	info.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
	info.pCommandBuffers = cmdBufs.data();
	VkResult result;
	if (queue == VK_NULL_HANDLE)
		result = vkQueueSubmit(_graphicsQueue, 1, &info, VK_NULL_HANDLE);
	else
		result = vkQueueSubmit(queue, 1, &info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
		MessageOut(" [Submit Queue Immediate]vkQueueSubmit error", false, false);
}

#include "PassBase.h"
void VulkanManager::SubmitQueueForPasses( std::vector<std::shared_ptr<PassBase>> passes, VkSemaphore presentSemaphore, VkPipelineStageFlags waitStageMask, VkQueue queue)
{
	std::vector<VkSubmitInfo> infos = {};
	std::vector<VkSemaphore> lastSem = { presentSemaphore };
	for (int i = 0; i < passes.size(); i++)
	{
		if (passes[i]->GetSemaphore() == VK_NULL_HANDLE)
		{
			CreateSemaphore(passes[i]->GetSemaphore());
		}
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pWaitDstStageMask = &waitStageMask;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &lastSem[i];
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &passes[i]->GetSemaphore();
		info.commandBufferCount = 1;
		info.pCommandBuffers = &passes[i]->GetCommandBuffer();
		infos.push_back(info);
		lastSem.push_back(passes[i]->GetSemaphore());
	}
	VkResult result;
	if (queue == VK_NULL_HANDLE)
		result = vkQueueSubmit(_graphicsQueue, (uint32_t)infos.size(), infos.data(), VK_NULL_HANDLE);
	else
		result = vkQueueSubmit(queue, (uint32_t)infos.size(), infos.data(), VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
		MessageOut(" [Submit Queue Immediate]vkQueueSubmit error", false, false);
}

HString VulkanManager::GetVkResult(VkResult code)
{
	HString text = "Code: ";
	switch (code)
	{
	case VK_SUCCESS:
		text += "VK_SUCCESS";
		break;
	case VK_NOT_READY:
		text += "VK_NOT_READY";
		break;
	case VK_TIMEOUT:
		text += "VK_TIMEOUT";
		break;
	case VK_EVENT_SET:
		text += "VK_EVENT_SET";
		break;
	case VK_EVENT_RESET:
		text += "VK_EVENT_RESET";
		break;
	case VK_INCOMPLETE:
		text += "VK_INCOMPLETE";
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		text += "VK_ERROR_OUT_OF_HOST_MEMORY";
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		text += "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		text += "VK_ERROR_INITIALIZATION_FAILED";
		break;
	case VK_ERROR_DEVICE_LOST:
		text += "VK_ERROR_DEVICE_LOST";
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		text += "VK_ERROR_MEMORY_MAP_FAILED";
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		text += "VK_ERROR_LAYER_NOT_PRESENT";
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		text += "VK_ERROR_EXTENSION_NOT_PRESENT";
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		text += "VK_ERROR_FEATURE_NOT_PRESENT";
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		text += "VK_ERROR_INCOMPATIBLE_DRIVER";
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		text += "VK_ERROR_TOO_MANY_OBJECTS";
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		text += "VK_ERROR_FORMAT_NOT_SUPPORTED";
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		text += "VK_ERROR_FRAGMENTED_POOL";
		break;
	case VK_ERROR_UNKNOWN:
		text += "VK_ERROR_UNKNOWN";
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		text += "VK_ERROR_OUT_OF_POOL_MEMORY";
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		text += "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		break;
	case VK_ERROR_FRAGMENTATION:
		text += "VK_ERROR_FRAGMENTATION";
		break;
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		text += "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		text += "VK_ERROR_SURFACE_LOST_KHR";
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		text += "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		break;
	case VK_SUBOPTIMAL_KHR:
		text += "VK_SUBOPTIMAL_KHR";
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		text += "VK_ERROR_OUT_OF_DATE_KHR";
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		text += "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		text += "VK_ERROR_VALIDATION_FAILED_EXT";
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		text += "VK_ERROR_INVALID_SHADER_NV";
		break;
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		text += "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		break;
	case VK_ERROR_NOT_PERMITTED_EXT:
		text += "VK_ERROR_NOT_PERMITTED_EXT";
		break;
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		text += "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		break;
	case VK_THREAD_IDLE_KHR:
		text += "VK_THREAD_IDLE_KHR";
		break;
	case VK_THREAD_DONE_KHR:
		text += "VK_THREAD_DONE_KHR";
		break;
	case VK_OPERATION_DEFERRED_KHR:
		text += "VK_OPERATION_DEFERRED_KHR";
		break;
	case VK_OPERATION_NOT_DEFERRED_KHR:
		text += "VK_OPERATION_NOT_DEFERRED_KHR";
		break;
	case VK_PIPELINE_COMPILE_REQUIRED_EXT:
		text += "VK_PIPELINE_COMPILE_REQUIRED_EXT";
		break;
	default:
		text = "[Unknow???]";
		break;
	}
	return text;
}
