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
#include "DescriptorSet.h"
#include "Primitive.h"

using namespace std;

std::unique_ptr<VulkanManager> VulkanManager::_vulkanManager;

PFN_vkCreateDebugReportCallbackEXT  fvkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT  fvkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

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
		if (!HString(msg).Contains("VkLayer_nsight-sys_windows.json")) //Ignore Nsight-sys json not found error.
		{
			bError = true;
		}
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		title = "DEBUG: "; color = "255,255,255";
	}
	//if (flags & VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT) {
	//	ConsoleDebug::print_endl(DString(": "));
	//}
	title = title + HString("@[") + layer_prefix + "]";
	ConsoleDebug::print_endl(title, color);
	ConsoleDebug::print_endl(msg, color);
	if(bError)
		MessageOut(HString(title + "\n" + msg).c_str(),false,true);
	return false;
}

// --------- IMGUI
#include "imgui.h"
#include "./backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_sdl3.h"
#if defined(_WIN32)
FILE* pFileOut;
FILE* pFileErr;
#endif

VulkanManager::VulkanManager(bool bDebug)
{
#if defined(_WIN32)
	_currentPlatform = EPlatform::Windows;
	if (bDebug)
	{
		AllocConsole();
		freopen_s(&pFileOut, "CONOUT$", "w", stdout);
		freopen_s(&pFileErr, "CONOUT$", "w", stderr);
	}
	_currentPlatform = EPlatform::Windows;
#elif defined(__ANDROID__)
	_currentPlatform = EPlatform::Android;
#elif defined(__linux__)
	_currentPlatform = EPlatform::Linux;
#endif

#if !defined(_WIN32)
	if (!InitVulkan())
	{
		// Vulkan 不可用
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Vulkan is not support on this device.", NULL);
		printf("Vulkan is not support on this device.");
		std::cout<< "Vulkan is not support on this device." <<std::endl;
		throw std::runtime_error("Vulkan is not support on this device.");
		exit(0);
	}
#endif
	ConsoleDebug::print_endl("hBBr:InitVulkan");
	ConsoleDebug::print_endl(RendererLauguage::GetText("T000000"));

	_bDebugEnable = false;
	_graphicsQueueFamilyIndex = -1;
	_swapchainBufferCount = 3;
	_enable_VK_KHR_display = false;
	ConsoleDebug::print_endl("hBBr:Start init Vulkan Instance.");
	//Init global vulkan  
	InitInstance(bDebug);
	ConsoleDebug::print_endl("hBBr:Start init Vulkan Device.");
	InitDevice();
	InitDebug();
	ConsoleDebug::print_endl("hBBr:Start init Command Pool.");
	CreateCommandPool();
	ConsoleDebug::print_endl("hBBr:Start Create Descripotr Pool.");
	CreateDescripotrPool(_descriptorPool);
	ConsoleDebug::print_endl("hBBr:Start init imgui.");
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

VulkanManager::~VulkanManager()
{
	ImGui::DestroyContext();
	DestroyDescriptorPool(_descriptorPool);
	DestroyCommandPool();
	if (_bDebugEnable)
		fvkDestroyDebugReportCallbackEXT(_instance, _debugReport, VK_NULL_HANDLE);
	if (_device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(_device, VK_NULL_HANDLE);
		_device = VK_NULL_HANDLE;
	}
	if (_instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(_instance, VK_NULL_HANDLE);
		_instance = VK_NULL_HANDLE;
	}
	#if defined(_WIN32)
		fclose(pFileOut);
		fclose(pFileErr);
	#endif

}

void VulkanManager::InitInstance(bool bEnableDebug)
{
	_bDebugEnable = bEnableDebug;
	//layers & extensions
	std::vector<const char*> extensions;// = { VK_KHR_SURFACE_EXTENSION_NAME };
	std::vector<const char*> layers;
	std::vector<VkLayerProperties> availableLaters;

	//列举支持的layers和extensions
	{
		uint32_t count;
		vkEnumerateInstanceLayerProperties(&count, VK_NULL_HANDLE);
		availableLaters.resize(count);
		vkEnumerateInstanceLayerProperties(&count, availableLaters.data());
		ConsoleDebug::print_endl("");
		ConsoleDebug::print_endl("");
		ConsoleDebug::print_endl("----------Enumerate Instance Layer Properties---------");
		std::vector<HString> layerLogs;

		for (uint32_t i = 0; i < count; i++)
		{
			bool bVK_LAYER_KHRONOS_validation = false;
			if (strcmp(availableLaters[i].layerName, "VK_LAYER_KHRONOS_validation") == 0)
			{
				layers.push_back("VK_LAYER_KHRONOS_validation");
				layerLogs.push_back("hBBr:[Vulkan Instance layer] Add VK_LAYER_KHRONOS_validation layer.");
				bVK_LAYER_KHRONOS_validation = true;
			}
			//RenderDoc支持
			//layers.push_back("VK_LAYER_RENDERDOC_Capture");//
			//_instance_layers.push_back(layerName);
			ConsoleDebug::print_endl(availableLaters[i].layerName + HString("  |  ") + availableLaters[i].description, "150,150,150");
			ConsoleDebug::print_endl("\t------------------");
		}
		ConsoleDebug::print_endl("\t---------End Enumerate Instance Layer Properties------");
		ConsoleDebug::print_endl("Found Instance Layers number:" + HString::FromSize_t(availableLaters.size()));
		//
		ConsoleDebug::print_endl("");
		ConsoleDebug::print_endl("");
		ConsoleDebug::print_endl("\t---------Enumerate Instance Extension Properties------");
		uint32_t ecount = 0;
		vkEnumerateInstanceExtensionProperties(NULL, &ecount, VK_NULL_HANDLE);
		std::vector<VkExtensionProperties> availableExts(ecount);
		vkEnumerateInstanceExtensionProperties(NULL, &ecount, availableExts.data());
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
			if (strcmp(availableExts[i].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				layerLogs.push_back("hBBr:[Vulkan Instance extension] Add VK_EXT_DEBUG_REPORT_EXTENSION_NAME ext.");

			}
			else if (strcmp(availableExts[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				layerLogs.push_back("hBBr:[Vulkan Instance extension] Add VK_EXT_DEBUG_UTILS_EXTENSION_NAME ext.");
			}
		}
		ConsoleDebug::print_endl("\t---------End Enumerate Instance Extension Properties------");

		for (auto i : layerLogs)
		{
			ConsoleDebug::print_endl(i);
		}	
	}

	//SDL
	{
		unsigned int eCount = 0 ;
		if(SDL_Vulkan_GetInstanceExtensions(&eCount, NULL) != SDL_TRUE)
		{
			MessageOut(SDL_GetError(), true, true, "255,0,0");
		}
		std::vector<const char*>sdlExts(eCount);
		if (SDL_Vulkan_GetInstanceExtensions(&eCount, sdlExts.data()) != SDL_TRUE)
		{
			MessageOut(SDL_GetError(), true, true, "255,0,0");
		}
		extensions.insert(extensions.end(), sdlExts.begin(), sdlExts.end());
	}
	
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = VK_NULL_HANDLE;
	appInfo.pApplicationName = "hBBr";
	appInfo.pEngineName = "hBBr Engine";
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = VK_NULL_HANDLE;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;

	if (_bDebugEnable)
	{
		ConsoleDebug::print_endl("hBBr:Enable Vulkan Debug layer.");

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

	VkResult result = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &_instance);
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
		vkEnumeratePhysicalDevices(_instance, &devicesCount, VK_NULL_HANDLE);
		std::vector<VkPhysicalDevice> gpu_list(devicesCount);
		vkEnumeratePhysicalDevices(_instance, &devicesCount, gpu_list.data());
		ConsoleDebug::print_endl("-----------GPU List-----------", "0,255,0");
		ConsoleDebug::print_endl("Found " + HString::FromUInt(devicesCount) + " GPU(s)", "222,255,255");
		//_gpuProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		for (uint32_t i = 0; i < devicesCount; i++)
		{
			//vkGetPhysicalDeviceProperties2(gpu_list[i], &_gpuProperties);
			vkGetPhysicalDeviceProperties(gpu_list[i], &_gpuProperties);
			ConsoleDebug::print_endl("Device Name:" + HString(_gpuProperties.deviceName), "200,255,255");
			ConsoleDebug::print_endl("Device ID:" + HString::FromUInt(_gpuProperties.deviceID), "150,150,150");

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
			_Sleep(500);
			MessageOut(RendererLauguage::GetText("A000002").c_str(),true, true);
		}
		//vkGetPhysicalDeviceProperties2(_gpuDevice, &_gpuProperties);
		vkGetPhysicalDeviceProperties(_gpuDevice, &_gpuProperties);
		vkGetPhysicalDeviceMemoryProperties(_gpuDevice, &_gpuMemoryProperties);
	}
	//------------------Queue Family
	ConsoleDebug::print_endl("Get Queue Family...", "0,255,0");
	VkQueueFamilyProperties graphicsQueueFamilyProperty{};
	//VkQueueFamilyProperties transferQueueFamilyProperty{};
	{
		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpuDevice, &family_count, VK_NULL_HANDLE);
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
	ConsoleDebug::print_endl("Get Device Layers and Extensions...", "0,255,0");
	std::vector <const char*> layers;
	std::vector <const char*> extensions;
	//列举Device支持的Layers和Extensions
	{
		std::vector<HString> layerLogs;
		uint32_t count = 0;
		vkEnumerateDeviceLayerProperties(_gpuDevice, &count, VK_NULL_HANDLE);
		std::vector<VkLayerProperties> availableLaters(count);
		vkEnumerateDeviceLayerProperties(_gpuDevice, &count, availableLaters.data());
		ConsoleDebug::print_endl("");
		ConsoleDebug::print_endl("");
		ConsoleDebug::print_endl("------------Enumerate Device Layer Properties-----------");
		for (uint32_t i = 0; i < count; i++) 
		{
			if (strcmp(availableLaters[i].layerName, "VK_LAYER_KHRONOS_validation") == 0 && _bDebugEnable)
			{
				layers.push_back("VK_LAYER_KHRONOS_validation");
				layerLogs.push_back("hBBr:[Vulkan Device layer] Add VK_LAYER_KHRONOS_validation layer.");
			}
			ConsoleDebug::print_endl(availableLaters[i].layerName + HString("  |  ") + availableLaters[i].description, "150,150,150");
			ConsoleDebug::print_endl("\t------------------");
		}
		ConsoleDebug::print_endl("----------End Enumerate Device Layer Properties---------");
		//
		ConsoleDebug::print_endl("");
		ConsoleDebug::print_endl("");
		ConsoleDebug::print_endl("\t---------Enumerate Device Extension Properties------");
		uint32_t ecount = 0;
		vkEnumerateDeviceExtensionProperties(_gpuDevice, NULL, &ecount, VK_NULL_HANDLE);
		std::vector<VkExtensionProperties> availableExts(ecount);
		vkEnumerateDeviceExtensionProperties(_gpuDevice, NULL, &ecount, availableExts.data());
		ConsoleDebug::print_endl("\tDevice Extension Properties---------");
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
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_debug_marker ext.");
					debugMarkerActive = true;
				}
				else if (strcmp(availableExts[i].extensionName, "VK_EXT_tooling_info") == 0)
				{
					extensions.push_back("VK_EXT_tooling_info");
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_tooling_info ext.");
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_SWAPCHAIN_EXTENSION_NAME ext.");
				}
				else if (strcmp(availableExts[i].extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME ext.");
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME ext.");
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME) == 0)
				{
					//允许深度/模板图像的图像存储屏障仅设置了深度或模板位之一，而不是两者都设置。
					extensions.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME ext.");
				}
			}
		}

		for (auto i : layerLogs)
		{
			ConsoleDebug::print_endl(i);
		}
		ConsoleDebug::print_endl("\t---------End Enumerate Device Extension Properties------");
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
	auto result = vkCreateDevice(_gpuDevice, &device_create_info, VK_NULL_HANDLE, &_device);
	if(result!= VK_SUCCESS) 
		MessageOut((RendererLauguage::GetText("A000004") + GetVkResult(result)).c_str() , true, true);
	vkGetDeviceQueue(_device, _graphicsQueueFamilyIndex, 0, &_graphicsQueue);
	//vkGetDeviceQueue(_device, _transferQueueFamilyIndex, 0, &_transfer_Queue);
#ifdef _WIN32
	if (_enable_VK_KHR_display)
	{
		uint32_t displayCount = 0;
		vkGetPhysicalDeviceDisplayPropertiesKHR(_gpuDevice, &displayCount, VK_NULL_HANDLE);
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
			vkGetDisplayModePropertiesKHR(_gpuDevice, displayPro[i].display, &count, VK_NULL_HANDLE);
			std::vector<VkDisplayModePropertiesKHR> displayMode(count);
			vkGetDisplayModePropertiesKHR(_gpuDevice, displayPro[i].display, &count, displayMode.data());
			for (uint32_t i = 0; i < count; i++)
			{
				ConsoleDebug::print_endl("Display Refresh Rate:" + HString::FromInt(displayMode[i].parameters.refreshRate), "200,255,255");
			}
		}
	}
#endif
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
		ConsoleDebug::print_endl("hBBr:Start Init Vulkan Debug.");
		fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
		fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
		if (VK_NULL_HANDLE == fvkCreateDebugReportCallbackEXT || VK_NULL_HANDLE == fvkDestroyDebugReportCallbackEXT)
		{
			MessageOut("Vulkan ERROR: Cant fetch debug function pointers.");
		}
		fvkCreateDebugReportCallbackEXT(_instance, &debugCallbackCreateInfo, VK_NULL_HANDLE, &_debugReport);
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
	//VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader; deviceFeatures.
	return deviceFeatures.tessellationShader && deviceFeatures.geometryShader;
}

void VulkanManager::CreateSurface_SDL(SDL_Window* handle, VkSurfaceKHR& newSurface)
{
	//SDL2
	if (SDL_Vulkan_CreateSurface(handle, _instance, &newSurface) == SDL_FALSE)
	{
		MessageOut("sdl Create Window Surface Failed.", true, true);
	}
}

void VulkanManager::DestroySurface(VkSurfaceKHR surface)
{
	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(_instance, surface, VK_NULL_HANDLE);
	}
}

VkExtent2D VulkanManager::GetSurfaceSize(VkExtent2D windowSize, VkSurfaceKHR surface)
{
	const int SwapchainBufferCount = 3;
	VkSurfaceCapabilitiesKHR _surfaceCapabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpuDevice, surface, &_surfaceCapabilities);
	if (_surfaceCapabilities.currentExtent.width < UINT32_MAX && _surfaceCapabilities.currentExtent.width>0) {
		windowSize.width = _surfaceCapabilities.currentExtent.width;
		windowSize.height = _surfaceCapabilities.currentExtent.height;
	}
	else {
		windowSize.width = _surfaceCapabilities.maxImageExtent.width < (uint32_t)windowSize.width ? _surfaceCapabilities.maxImageExtent.width : (uint32_t)windowSize.width;
		windowSize.width = _surfaceCapabilities.minImageExtent.width > (uint32_t)windowSize.width ? _surfaceCapabilities.minImageExtent.width : (uint32_t)windowSize.width;
		windowSize.height = _surfaceCapabilities.maxImageExtent.height < (uint32_t)windowSize.height ? _surfaceCapabilities.maxImageExtent.height : (uint32_t)windowSize.height;
		windowSize.height = _surfaceCapabilities.minImageExtent.height > (uint32_t)windowSize.height ? _surfaceCapabilities.minImageExtent.height : (uint32_t)windowSize.height;
	}
	_swapchainBufferCount = _surfaceCapabilities.minImageCount > SwapchainBufferCount ? _surfaceCapabilities.minImageCount : SwapchainBufferCount;
	_swapchainBufferCount = _surfaceCapabilities.maxImageCount < _swapchainBufferCount ? _surfaceCapabilities.maxImageCount : _swapchainBufferCount;
	return windowSize;
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

VkExtent2D VulkanManager::CreateSwapchain(VkExtent2D surfaceSize, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat , VkSwapchainKHR &newSwapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews)
{
	//ConsoleDebug::print_endl("Create Swapchain KHR.");
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	//if (_winInfo.vsync)
	{
		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, VK_NULL_HANDLE);
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
	VkExtent2D _surfaceSize = GetSurfaceSize(surfaceSize,surface);
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
	info.pQueueFamilyIndices = VK_NULL_HANDLE;
	info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//是否半透明，用于组合其他表面,这里我们不需要
	info.presentMode = present_mode;
	info.clipped = VK_TRUE;//是否不渲染看不见的位置
	//替补swapchain,在重置这个swapchain的时候会比较有用，但是事实上我们可以用更暴力的方法重置swapchain，也就是完全重写
	info.oldSwapchain = VK_NULL_HANDLE;
	auto result = vkCreateSwapchainKHR(_device, &info, VK_NULL_HANDLE, &newSwapchain);
	if (result != VK_SUCCESS)
	{
		MessageOut((RendererLauguage::GetText("A000007").c_str() + GetVkResult(result)).c_str(), false, true);
	}
	vkGetSwapchainImagesKHR(_device, newSwapchain, &_swapchainBufferCount, VK_NULL_HANDLE);
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
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	FreeCommandBuffers(_commandPool, { buf });
	return _surfaceSize;
}

VkExtent2D VulkanManager::CreateSwapchain(VkExtent2D surfaceSize, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSwapchainKHR& newSwapchain, std::vector<std::shared_ptr<class Texture>>& textures, std::vector<VkImageView>& swapchainImageViews)
{
	//ConsoleDebug::print_endl("Create Swapchain KHR.");
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	//if (_winInfo.vsync)
	{
		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, VK_NULL_HANDLE);
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
	VkExtent2D _surfaceSize = GetSurfaceSize(surfaceSize, surface);
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
	info.pQueueFamilyIndices = VK_NULL_HANDLE;
	info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//是否半透明，用于组合其他表面,这里我们不需要
	info.presentMode = present_mode;
	info.clipped = VK_TRUE;//是否不渲染看不见的位置
	//替补swapchain,在重置这个swapchain的时候会比较有用，但是事实上我们可以用更暴力的方法重置swapchain，也就是完全重写
	info.oldSwapchain = VK_NULL_HANDLE;
	auto result = vkCreateSwapchainKHR(_device, &info, VK_NULL_HANDLE, &newSwapchain);
	if (result != VK_SUCCESS)
	{
		MessageOut((RendererLauguage::GetText("A000007").c_str() + GetVkResult(result)).c_str(), true, true);
	}
	vkGetSwapchainImagesKHR(_device, newSwapchain, &_swapchainBufferCount, VK_NULL_HANDLE);
	textures.resize(_swapchainBufferCount);
	std::vector<VkImage> images(_swapchainBufferCount);
	vkGetSwapchainImagesKHR(_device, newSwapchain, &_swapchainBufferCount, images.data());
	//创建ImageView
	swapchainImageViews.clear();
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		textures[i].reset(new Texture(true));
		textures[i]->_image = images[i];
		textures[i]->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		textures[i]->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		textures[i]->_format = surfaceFormat.format;
		textures[i]->_usageFlags = info.imageUsage;
		textures[i]->_textureName = "Swapchain Image " + HString::FromInt(i);
		CreateImageView(textures[i]->_image, surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, textures[i]->_imageView);
		swapchainImageViews.push_back(textures[i]->_imageView);
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
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	FreeCommandBuffers(_commandPool, { buf });
	return _surfaceSize;
}

void VulkanManager::DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<VkImageView>& swapchainImageViews)
{
	for (int i = 0; i < (int)swapchainImageViews.size(); i++)
	{
		DestroyImageView(swapchainImageViews[i]);
	}
	swapchainImageViews.clear();
	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_device, swapchain, VK_NULL_HANDLE);
		swapchain = VK_NULL_HANDLE;
	}
}

void VulkanManager::DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<std::shared_ptr<class Texture>>& textures)
{
	for (int i = 0; i < (int)textures.size(); i++)
	{
		DestroyImageView(textures[i]->_imageView);
		textures[i]->_image = VK_NULL_HANDLE;
		textures[i]->_imageView = VK_NULL_HANDLE;
	}
	textures.clear();
	//
	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_device, swapchain, VK_NULL_HANDLE);
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
	create_info.pQueueFamilyIndices = VK_NULL_HANDLE;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	auto result = vkCreateImage(_device, &create_info, VK_NULL_HANDLE, &image);
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
	vkCreateImageView(_device, &image_view_create_info, VK_NULL_HANDLE, &imageView);
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
	auto err = vkAllocateMemory(_device, &memory_allocate_info, VK_NULL_HANDLE, &imageViewMemory);
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
	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
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
	default:
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
		dstFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageBarrier.dstAccessMask |=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dstFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dstFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;
	default:
		break;
	}
	vkCmdPipelineBarrier(cmdBuffer, srcFlags, dstFlags, 0, 0, NULL, 0, NULL, 1,
		&imageBarrier);
}

void VulkanManager::DestroyImage(VkImage& image)
{
	if (image != VK_NULL_HANDLE)
	{
		vkDestroyImage(_device, image, VK_NULL_HANDLE);
	}
}

void VulkanManager::DestroyImageMemory(VkDeviceMemory& imageViewMemory)
{
	if (imageViewMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(_device, imageViewMemory, VK_NULL_HANDLE);
	}
}

void VulkanManager::DestroyImageView(VkImageView& imageView)
{
	if (imageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(_device, imageView, VK_NULL_HANDLE);
	}
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

	vkCreateCommandPool(_device, &cmdPoolInfo, VK_NULL_HANDLE, &commandPool);
}

void VulkanManager::DestroyCommandPool(VkCommandPool commandPool)
{
	if (commandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(_device, commandPool, VK_NULL_HANDLE);
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

void VulkanManager::ResetCommandBuffer(VkCommandBuffer cmdBuf)
{
	vkResetCommandBuffer(cmdBuf, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
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
	info.pInheritanceInfo = VK_NULL_HANDLE;//继承,不用
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
			clearValue.depthStencil.depth = 1.0f;
			clearValue.depthStencil.stencil = 0;
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

bool VulkanManager::GetNextSwapchainIndex(VkSwapchainKHR swapchain, VkSemaphore semaphore, uint32_t* swapchainIndex)
{
	VkResult result = vkAcquireNextImageKHR(_device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, swapchainIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		#if _DEBUG
		//MessageOut(RendererLauguage::GetText("A000009").c_str(), false, false);//太烦人了,不显示了,反正不影响
		#endif	
		return false;
	}
	else if (result != VK_SUCCESS)
	{
		MessageOut(RendererLauguage::GetText("A000010").c_str(), false, true);
		return false;
	}
	return true;
}

bool VulkanManager::Present(VkSwapchainKHR swapchain, VkSemaphore semaphore, uint32_t& swapchainImageIndex)
{
	VkResult infoResult = VK_SUCCESS;
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &swapchainImageIndex;
	presentInfo.pResults = &infoResult; // Optional
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphore;
	auto result = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		#if _DEBUG
		//MessageOut("VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR.Swapchain need to reset.", false, false);//太烦人了,不显示了,反正不影响
		#endif
		return false;
	}
	else if (result != VK_SUCCESS || infoResult != VK_SUCCESS)
	{
		MessageOut(RendererLauguage::GetText("A000011").c_str(), false, true);
		return false;
	}
	return true;
}

void VulkanManager::CreatePipelineLayout(std::vector <VkDescriptorSetLayout> descriptorSetLayout, VkPipelineLayout& pipelineLayout)
{
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.flags = 0;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = VK_NULL_HANDLE;
	info.pSetLayouts = descriptorSetLayout.data();
	info.setLayoutCount = (uint32_t)descriptorSetLayout.size();
	auto result = vkCreatePipelineLayout(_device, &info, VK_NULL_HANDLE, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		MessageOut("vkCreatePipelineLayout error!",true,true);
	}
}

void VulkanManager::DestroyPipelineLayout(VkPipelineLayout& pipelineLayout)
{
	if (pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(_device, pipelineLayout, VK_NULL_HANDLE);
	}
	pipelineLayout = VK_NULL_HANDLE;
}

void VulkanManager::CreateDescripotrPool(VkDescriptorPool& pool)
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },	//这是一个image和sampler的组合descriptor
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },			//纯image,不带sampler
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
	vkCreateDescriptorPool(_device, &info, VK_NULL_HANDLE, &pool);
}

void VulkanManager::DestroyDescriptorPool(VkDescriptorPool& pool)
{
	if (pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(_device, pool, VK_NULL_HANDLE);
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
		bindings[i].pImmutableSamplers = VK_NULL_HANDLE;
	}
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = (uint32_t)bindings.size();
	info.pBindings = bindings.data();
	auto result = vkCreateDescriptorSetLayout(_device, &info, VK_NULL_HANDLE, &descriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		MessageOut("Create Descriptor Set Layout Error!!", false, true);
	}	
}

void VulkanManager::CreateDescripotrSetLayout(std::vector<VkDescriptorType> types, VkDescriptorSetLayout& descriptorSetLayout, VkShaderStageFlags shaderStageFlags)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(types.size());
	for (uint32_t i = 0; i < types.size(); i++)
	{
		bindings[i] = {};
		bindings[i].descriptorType = types[i];
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = shaderStageFlags;
		bindings[i].binding = i;
		bindings[i].pImmutableSamplers = VK_NULL_HANDLE;
	}
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = (uint32_t)bindings.size();
	info.pBindings = bindings.data();
	auto result = vkCreateDescriptorSetLayout(_device, &info, VK_NULL_HANDLE, &descriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		MessageOut("Create Descriptor Set Layout Error!!", false, true);
	}
}

void VulkanManager::DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
	if (descriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(_device, descriptorSetLayout, VK_NULL_HANDLE);
	}
}

void VulkanManager::CreateFrameBuffers(VkExtent2D FrameBufferSize, VkRenderPass renderPass, std::vector<VkImageView> attachments, std::vector<VkFramebuffer>& frameBuffers)
{
	frameBuffers.resize(_swapchainBufferCount);
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = (uint32_t)attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = FrameBufferSize.width;
		framebufferInfo.height = FrameBufferSize.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(_device, &framebufferInfo, VK_NULL_HANDLE, &frameBuffers[i]) != VK_SUCCESS) {
			printf("failed to create framebuffer!");
			throw std::runtime_error("failed to create framebuffer!");
			exit(0);
		}
	}
}

void VulkanManager::DestroyFrameBuffers(std::vector<VkFramebuffer>& frameBuffers)
{
	for (int i = 0; i < frameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(_device, frameBuffers[i], VK_NULL_HANDLE);
	}
	frameBuffers.clear();
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
	auto result = vkCreateFramebuffer(_device, &info, VK_NULL_HANDLE, &framebuffer);
	if (result != VK_SUCCESS)
	{
		MessageOut("Vulkan ERROR: Create Frame Buffer Failed.", false, true);
	}
}

void VulkanManager::DestroyFrameBuffer(VkFramebuffer& framebuffer)
{
	if (framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(_device, framebuffer, VK_NULL_HANDLE);
		framebuffer = VK_NULL_HANDLE;
	}
}

void VulkanManager::AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet)
{
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &descriptorSetLayout;
	auto result = vkAllocateDescriptorSets(_device, &info, &descriptorSet);
	if (result != VK_SUCCESS)
	{
		MessageOut("Vulkan ERROR: Allocate Descriptor Sets Failed.", false, true);
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
	semaphoreCreateInfo.pNext = VK_NULL_HANDLE;
	vkCreateSemaphore(_device, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore);
}

void VulkanManager::DestroySemaphore(VkSemaphore& semaphore)
{
	if (semaphore  != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(_device, semaphore, VK_NULL_HANDLE);
	}
	semaphore = VK_NULL_HANDLE;
}

void VulkanManager::CreateRenderSemaphores(std::vector<VkSemaphore>& semaphore)
{
	semaphore.resize(_swapchainBufferCount);
	for (int i = 0; i < semaphore.size(); i++)
	{
		this->CreateSemaphore(semaphore[i]);
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

void VulkanManager::CreateFence(VkFence& fence)
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.flags = VK_FENCE_CREATE_SIGNALED_BIT;//default signaled
	VkResult result = vkCreateFence(_device, &info, VK_NULL_HANDLE, &fence);
	if (result != VK_SUCCESS) {
		MessageOut("Create Fence Failed.", false, true);
	}
}

void VulkanManager::DestroyFence(VkFence& fence)
{
	if (fence != VK_NULL_HANDLE)
	{
		vkDestroyFence(_device, fence, VK_NULL_HANDLE);
		fence = VK_NULL_HANDLE;
	}
}

void VulkanManager::CreateRenderFences(std::vector<VkFence>& fences)
{
	fences.resize(_swapchainBufferCount);
	for (int i = 0; i < fences.size(); i++)
	{
		this->CreateFence(fences[i]);
	}
}

void VulkanManager::DestroyRenderFences(std::vector<VkFence>& fences)
{
	for (int i = 0; i < fences.size(); i++)
	{
		if (fences[i] != VK_NULL_HANDLE)
		{
			DestroyFence(fences[i]);
		}
	}
	fences.clear();
}

void VulkanManager::WaitForFences(std::vector<VkFence> fences, bool bReset, uint64_t timeOut)
{
	vkWaitForFences(_device, (uint32_t)fences.size(), fences.data(), VK_TRUE, timeOut);
	if (bReset)
		vkResetFences(_device, (uint32_t)fences.size(), fences.data());
}

//void VulkanManager::WaitSemaphores(std::vector<VkSemaphore> semaphores, uint64_t timeOut)
//{
//	//Semaphore 标识,借助vkGetSemaphoreCounterValue 用来查询信号状态用的，返回的value是否等于我们这里指定的值，是就代表获取到信号。
//	//不具备实际数据作用,如果没有查询的意义,随便给一个不同的值就行了,但不能为NULL(0)
//	std::vector<uint64_t> values(semaphores.size());
//	for (int i = 0 ; i < (int)values.size();i++)
//	{
//		values[i] = i + (uint64_t)1;
//	}
//	VkSemaphoreWaitInfo waitInfo = {};
//	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
//	waitInfo.semaphoreCount = (uint32_t)semaphores.size();
//	waitInfo.pSemaphores = semaphores.data();
//	waitInfo.pValues = values.data();
//	vkWaitSemaphores(_device, &waitInfo, timeOut);
//}

void VulkanManager::CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo& info, VkPipeline& pipeline)
{
	auto result = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &info, VK_NULL_HANDLE, &pipeline);
	if (result != VK_SUCCESS)
	{
		MessageOut(RendererLauguage::GetText("A000012").c_str(), true, true);
	}
}

void VulkanManager::CreateRenderPass(std::vector<VkAttachmentDescription>attachmentDescs, std::vector<VkSubpassDependency>subpassDependencys, std::vector<VkSubpassDescription>subpassDescs, VkRenderPass& renderPass)
{
	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.flags = 0;
	info.attachmentCount = (uint32_t)attachmentDescs.size();
	info.pAttachments = attachmentDescs.data();
	info.dependencyCount = (uint32_t)subpassDependencys.size();
	info.pDependencies = subpassDependencys.data();
	info.pNext = NULL;
	info.subpassCount = (uint32_t)subpassDescs.size();
	info.pSubpasses = subpassDescs.data();
	vkCreateRenderPass(_device, &info, VK_NULL_HANDLE, &renderPass);
}

void VulkanManager::DestroyRenderPass(VkRenderPass& renderPass)
{
	if (renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(_device, renderPass, VK_NULL_HANDLE);
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
	if (vkCreateBuffer(_device, &create_info, VK_NULL_HANDLE, &buffer) != VK_SUCCESS) {
		MessageOut("[ Create Buffer ] Create Uniform Buffer Failed.", true, true, "255,2,0");
	}
}


void VulkanManager::DestroyBuffer(VkBuffer& buffer)
{
	if (buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(_device, buffer, VK_NULL_HANDLE);
		buffer = VK_NULL_HANDLE;
	}
}

void VulkanManager::AllocateBufferMemory(VkBuffer buffer, VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags propertyFlags)
{
	VkMemoryRequirements mem_requirement;
	vkGetBufferMemoryRequirements(_device, buffer, &mem_requirement);
	VkMemoryAllocateInfo mem_allocate_info{};
	mem_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_allocate_info.allocationSize = mem_requirement.size;
	mem_allocate_info.memoryTypeIndex = FindMemoryTypeIndex(&mem_requirement, propertyFlags);
	if (vkAllocateMemory(_device, &mem_allocate_info, VK_NULL_HANDLE, &bufferMemory) != VK_SUCCESS) {
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
		vkFreeMemory(_device, bufferMemory, VK_NULL_HANDLE);
	}
}

void VulkanManager::CreateShaderModule(std::vector<char> data, VkShaderModule& shaderModule)
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = data.size();
	info.pCode = reinterpret_cast<const uint32_t*>(data.data());
	vkCreateShaderModule(_device, &info, VK_NULL_HANDLE, &shaderModule);
}

void VulkanManager::CreateShaderModule(VkDevice device, std::vector<char> data, VkShaderModule& shaderModule)
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = data.size();
	info.pCode = reinterpret_cast<const uint32_t*>(data.data());
	vkCreateShaderModule(device, &info, VK_NULL_HANDLE, &shaderModule);
}

void VulkanManager::InitImgui_SDL(SDL_Window* handle, VkRenderPass renderPass, uint32_t subPassIndex)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;

	if (!ImGui_ImplSDL3_InitForVulkan(handle))
		MessageOut("Error,ImGui_ImplSDL3_InitForVulkan return false!",true,true,"255,0,0");

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = _instance;
	init_info.PhysicalDevice = _gpuDevice;
	init_info.Device = _device;
	init_info.QueueFamily = _graphicsQueueFamilyIndex;
	init_info.Queue = _graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = _descriptorPool;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.MinImageCount = _swapchainBufferCount;
	init_info.ImageCount = _swapchainBufferCount;
	init_info.Subpass = subPassIndex;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;
	//init_info.UseDynamicRendering = true;
	ImGui_ImplVulkan_Init(&init_info, renderPass);

	VkCommandBuffer buf;
	AllocateCommandBuffer(_commandPool, buf);
	BeginCommandBuffer(buf, 0);
	ImGui_ImplVulkan_CreateFontsTexture(buf);
	EndCommandBuffer(buf);
	SubmitQueueImmediate({ buf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	FreeCommandBuffers(_commandPool, { buf });
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanManager::ShutdownImgui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
}

void VulkanManager::ImguiNewFrame()
{
	ImGui_ImplVulkan_SetMinImageCount(_swapchainBufferCount);
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void VulkanManager::ImguiEndFrame(VkCommandBuffer cmdBuf)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
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
		MessageOut((HString("[Submit Queue]vkQueueSubmit error : ")+ GetVkResult(result)).c_str(), false, true);
}

VkViewport VulkanManager::GetViewport(float w, float h)
{
	VkViewport viewport = {};
	viewport.x = 0.0f; // 视口左下角的x坐标
	viewport.y = 0.0f; // 视口左下角的y坐标
	viewport.width = (float)w; // 视口的宽度
	viewport.height = (float)h; // 视口的高度
	viewport.minDepth = 0.0f; // 视口的最小深度
	viewport.maxDepth = 1.0f; // 视口的最大深度
	return viewport;
}

#include "PassBase.h"
void VulkanManager::SubmitQueueForPasses(VkCommandBuffer cmdBuf, std::vector<std::shared_ptr<PassBase>> passes, VkSemaphore presentSemaphore, VkSemaphore submitFinishSemaphore, VkFence executeFence, VkPipelineStageFlags waitStageMask, VkQueue queue)
{
	VkSemaphore* lastSem = &presentSemaphore;
	//std::vector<VkSubmitInfo> infos(passes.size());
	//for (int i = 0; i < passes.size(); i++)
	//{
	//	if (passes[i]->GetSemaphore() == VK_NULL_HANDLE)
	//	{
	//		this->CreateSemaphore(passes[i]->GetSemaphore());
	//	}
	//	infos[i].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//	infos[i].pWaitDstStageMask = &waitStageMask;
	//	infos[i].waitSemaphoreCount = 1;
	//	infos[i].pWaitSemaphores = lastSem;
	//	infos[i].signalSemaphoreCount = 1;
	//	infos[i].pSignalSemaphores = &passes[i]->GetSemaphore();
	//	infos[i].commandBufferCount = 1;
	//	infos[i].pCommandBuffers = &cmdBuf;
	//	lastSem = &passes[i]->GetSemaphore();
	//}
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pWaitDstStageMask = &waitStageMask;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = lastSem;
	info.signalSemaphoreCount = 1;
	info.pSignalSemaphores = &submitFinishSemaphore;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &cmdBuf;
	VkResult result;
	if (queue == VK_NULL_HANDLE)
		result = vkQueueSubmit(_graphicsQueue, (uint32_t)1, &info, executeFence);
	else
		result = vkQueueSubmit(queue, (uint32_t)1, &info, executeFence);
	if (result != VK_SUCCESS)
		MessageOut((HString("[Submit Queue]vkQueueSubmit error : ") + GetVkResult(result)).c_str(), false, true);
}

void VulkanManager::UpdateBufferDescriptorSet(class DescriptorSet* descriptorSet, uint32_t dstBinding, VkDeviceSize offset, VkDeviceSize Range)
{
	for (int i = 0; i < descriptorSet->GetTypes().size(); i++)
	{
		if (descriptorSet->GetTypes()[i] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || descriptorSet->GetTypes()[i] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = descriptorSet->GetBuffer()->GetBuffer();
			bufferInfo.offset = offset;
			bufferInfo.range = Range;
			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet->GetDescriptorSet();
			descriptorWrite.dstBinding = dstBinding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = descriptorSet->GetTypes()[i];
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = VK_NULL_HANDLE; // Optional
			descriptorWrite.pTexelBufferView = VK_NULL_HANDLE; // Optional
			vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, VK_NULL_HANDLE);
		}
	}
}

void VulkanManager::UpdateBufferDescriptorSet(DescriptorSet* descriptorSet, uint32_t dstBinding, uint32_t sameBufferSize, std::vector<uint32_t> offsets)
{
	for (int i = 0; i < descriptorSet->GetTypes().size(); i++)
	{
		if (descriptorSet->GetTypes()[i] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || descriptorSet->GetTypes()[i] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			std::vector<VkWriteDescriptorSet> descriptorWrite(offsets.size());
			std::vector<VkDescriptorBufferInfo> bufferInfo(offsets.size());
			for (int o = 0; o < offsets.size(); o++)
			{
				bufferInfo[o] = {};
				bufferInfo[o].buffer = descriptorSet->GetBuffer()->GetBuffer();
				bufferInfo[o].offset = offsets[o];
				bufferInfo[o].range = (VkDeviceSize)sameBufferSize;
				descriptorWrite[o] = {};
				descriptorWrite[o].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[o].dstSet = descriptorSet->GetDescriptorSet();
				descriptorWrite[o].dstBinding = dstBinding;
				descriptorWrite[o].dstArrayElement = 0;
				descriptorWrite[o].descriptorType = descriptorSet->GetTypes()[i];
				descriptorWrite[o].descriptorCount = 1;
				descriptorWrite[o].pBufferInfo = &bufferInfo[o];
				descriptorWrite[o].pImageInfo = VK_NULL_HANDLE; // Optional
				descriptorWrite[o].pTexelBufferView = VK_NULL_HANDLE; // Optional
			}
			vkUpdateDescriptorSets(_device, (uint32_t)offsets.size() , descriptorWrite.data(), 0, VK_NULL_HANDLE);
		}
	}
}

void VulkanManager::UpdateBufferDescriptorSet(DescriptorSet* descriptorSet, uint32_t dstBinding, std::vector<uint32_t> bufferSizes, std::vector<uint32_t> offsets)
{
	for (int i = 0; i < descriptorSet->GetTypes().size(); i++)
	{
		if (descriptorSet->GetTypes()[i] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || descriptorSet->GetTypes()[i] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			std::vector<VkWriteDescriptorSet> descriptorWrite(offsets.size());
			std::vector<VkDescriptorBufferInfo> bufferInfo(offsets.size());
			for (int o = 0; o < offsets.size(); o++)
			{
				bufferInfo[o] = {};
				bufferInfo[o].buffer = descriptorSet->GetBuffer()->GetBuffer();
				bufferInfo[o].offset = offsets[o];
				bufferInfo[o].range = bufferSizes[o];
				descriptorWrite[o] = {};
				descriptorWrite[o].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[o].dstSet = descriptorSet->GetDescriptorSet();
				descriptorWrite[o].dstBinding = dstBinding;
				descriptorWrite[o].dstArrayElement = 0;
				descriptorWrite[o].descriptorType = descriptorSet->GetTypes()[i];
				descriptorWrite[o].descriptorCount = 1;
				descriptorWrite[o].pBufferInfo = &bufferInfo[o];
				descriptorWrite[o].pImageInfo = VK_NULL_HANDLE; // Optional
				descriptorWrite[o].pTexelBufferView = VK_NULL_HANDLE; // Optional
			}
			vkUpdateDescriptorSets(_device, (uint32_t)offsets.size(), descriptorWrite.data(), 0, VK_NULL_HANDLE);
		}
	}
}

void VulkanManager::UpdateBufferDescriptorSetAll(DescriptorSet* descriptorSet, uint32_t dstBinding, VkDeviceSize offset, VkDeviceSize Range)
{
	for (int i = 0; i < descriptorSet->GetTypes().size(); i++)
	{
		if (descriptorSet->GetTypes()[i] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || descriptorSet->GetTypes()[i] & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			std::vector<VkDescriptorBufferInfo> bufferInfo(_swapchainBufferCount);
			std::vector<VkWriteDescriptorSet> descriptorWrite(_swapchainBufferCount);
			for (uint32_t d = 0; d < _swapchainBufferCount; d++)
			{
				bufferInfo[d] = {};
				bufferInfo[d].buffer = descriptorSet->GetBuffer()->GetBuffer();
				bufferInfo[d].offset = offset;
				bufferInfo[d].range = Range;
				descriptorWrite[d] = {};
				descriptorWrite[d].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[d].dstSet = descriptorSet->GetDescriptorSet(d);
				descriptorWrite[d].dstBinding = dstBinding;
				descriptorWrite[d].dstArrayElement = 0;
				descriptorWrite[d].descriptorType = descriptorSet->GetTypes()[i];
				descriptorWrite[d].descriptorCount = 1;
				descriptorWrite[d].pBufferInfo = &bufferInfo[d];
				descriptorWrite[d].pImageInfo = VK_NULL_HANDLE; // Optional
				descriptorWrite[d].pTexelBufferView = VK_NULL_HANDLE; // Optional
			}		
			vkUpdateDescriptorSets(_device, _swapchainBufferCount, descriptorWrite.data(), 0, VK_NULL_HANDLE);
		}
	}
}

VkDeviceSize VulkanManager::GetMinUboAlignmentSize(VkDeviceSize realSize)
{
	VkDeviceSize outSize = realSize;
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(_gpuDevice, &properties);
	VkDeviceSize minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;
	if (minUboAlignment > 0) {
		outSize = (outSize + (VkDeviceSize)minUboAlignment - 1) & ~((VkDeviceSize)minUboAlignment - 1);
	}
	return outSize;
}

void VulkanManager::UpdateImageSamplerDescriptorSet(DescriptorSet* descriptorSet, uint32_t dstBinding, VkDeviceSize offset, VkDeviceSize Range)
{

}

void VulkanManager::CmdSetViewport(VkCommandBuffer cmdbuf, std::vector<VkExtent2D> viewports)
{
	std::vector<VkViewport> vps(viewports.size());
	std::vector<VkRect2D> scissors(viewports.size());
	for (int i = 0; i < vps.size(); i++)
	{
		vps[i] = GetViewport((float)viewports[i].width, (float)viewports[i].height);
		scissors[i].extent = { viewports[i].width , viewports[i].height };
	}
	vkCmdSetViewport(cmdbuf, 0, (uint32_t)viewports.size(), vps.data());
	vkCmdSetScissor(cmdbuf, 0, (uint32_t)scissors.size(), scissors.data());
}

void VulkanManager::CmdNextSubpass(VkCommandBuffer cmdbuf, VkSubpassContents subpassContents)
{
	vkCmdNextSubpass(cmdbuf, subpassContents);
}

void VulkanManager::CmdCmdBindPipeline(VkCommandBuffer cmdbuf, VkPipeline pipelineObject, VkPipelineBindPoint bindPoint)
{
	vkCmdBindPipeline(cmdbuf, bindPoint, pipelineObject);
}

void VulkanManager::SetObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char* name)
{
	if (VulkanManager::debugMarkerActive)
	{
		VkDebugMarkerObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = objectType;
		nameInfo.object = object;
		nameInfo.pObjectName = name;
		VulkanManager::vkDebugMarkerSetObjectName(device, &nameInfo);
	}
}

void VulkanManager::SetObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag)
{
	if (VulkanManager::debugMarkerActive)
	{
		VkDebugMarkerObjectTagInfoEXT tagInfo = {};
		tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
		tagInfo.objectType = objectType;
		tagInfo.object = object;
		tagInfo.tagName = name;
		tagInfo.tagSize = tagSize;
		tagInfo.pTag = tag;
		VulkanManager::vkDebugMarkerSetObjectTag(device, &tagInfo);
	}
}

void VulkanManager::BeginRegion(VkCommandBuffer cmdbuf, const char* pMarkerName, glm::vec4 color)
{
	if (VulkanManager::debugMarkerActive)
	{
		VkDebugMarkerMarkerInfoEXT markerInfo = {};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
		markerInfo.pMarkerName = pMarkerName;
		VulkanManager::vkCmdDebugMarkerBegin(cmdbuf, &markerInfo);
	}
}

void VulkanManager::InsertRegion(VkCommandBuffer cmdbuf, std::string markerName, glm::vec4 color)
{
	if (VulkanManager::debugMarkerActive)
	{
		VkDebugMarkerMarkerInfoEXT markerInfo = {};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		memcpy(markerInfo.color, &color[0], sizeof(float) * 4);
		markerInfo.pMarkerName = markerName.c_str();
		VulkanManager::vkCmdDebugMarkerInsert(cmdbuf, &markerInfo);
	}
}

void VulkanManager::EndRegion(VkCommandBuffer cmdbuf)
{
	if (VulkanManager::vkCmdDebugMarkerEnd)
	{
		VulkanManager::vkCmdDebugMarkerEnd(cmdbuf);
	}
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