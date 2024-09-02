//Vulkan 内存管理器
#define VMA_IMPLEMENTATION

#if __ANDROID__
#define API_VERSION  VK_API_VERSION_1_1;
#define VMA_VULKAN_VERSION 1001000
//#define VMA_STATIC_VULKAN_FUNCTIONS 0 //安卓走的是动态库
//#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#else
#define API_VERSION  VK_API_VERSION_1_3;
#define VMA_VULKAN_VERSION 1003000
#endif

//
#include "VulkanManager.h"
#include <vector>
#include <array>
#include "HString.h"
#include "ConsoleDebug.h"
#include "RendererConfig.h"
#include "Texture2D.h"
#include "FileSystem.h"
#include "Shader.h"
#include "DescriptorSet.h"
#include "Primitive.h"
#include "Pass/PassBase.h"
#include "FormMain.h"

// --------- IMGUI
#if ENABLE_IMGUI
#include "Imgui/imgui.h"
#include "Imgui/backends/imgui_impl_sdl3.h"
#include "Imgui/backends/imgui_impl_vulkan.h"
#endif

std::unique_ptr<VulkanManager> VulkanManager::_vulkanManager;

PFN_vkCreateDebugReportCallbackEXT  fvkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT  fvkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;
bool VulkanManager::_bDebugEnable= false;
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
	bool bInformation = false;
	bool bDebug = false;
	bool bError = false;
	bool bWarning = false;
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		title = "INFO: \n"; color = "255,255,255";
		bInformation = true;
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		title = "WARNING: \n"; color = "255,255,0";
		bWarning = true;
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		title = "PERFORMANCE WARNING: \n"; color = "255,175,0";
		bWarning = true;
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		title = "ERROR: \n"; color = "255,0,0";
		bError = true;
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		title = "DEBUG: \n"; color = "200,200,200";
		bDebug = true;
	}
	title = title + HString("@[") + layer_prefix + "]";
	if (bError)
	{
		try {
			MessageOut(HString(title + msg), false, true, "255,0,0");
		}
		catch(const std::system_error& e)
		{
			ConsoleDebug::print_endl(HString("Error: ")+ e.what());
		}
	}
	else if (bDebug)
	{
		MessageOut(HString(title + msg), false, false, "255,255,0");
	}
	else if (bInformation)
	{
		MessageOut(HString(title + msg), false, false, "255,255,0");
	}
	else if(bWarning)
	{
		MessageOut(HString(title + msg), false, false, "255,255,0");
	}
	return false;
}

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
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Vulkan is not support on this device.", nullptr);
		printf("Vulkan is not support on this device.");
		std::cout<< "Vulkan is not support on this device." <<std::endl;
		throw std::runtime_error("Vulkan is not support on this device.");
	}
#endif
	ConsoleDebug::print_endl("hBBr:InitVulkan");
	ConsoleDebug::print_endl(GetInternationalizationText("Renderer", "T000000"));

	_bDebugEnable = false;
	_graphicsQueueFamilyIndex = -1;
	_swapchainBufferCount = 3;
	ConsoleDebug::print_endl("hBBr:Start init Vulkan Instance.");
	//Init global vulkan  
	InitInstance(bDebug);
	ConsoleDebug::print_endl("hBBr:Start init Vulkan Device.");
	InitDevice();
	InitDebug();
	ConsoleDebug::print_endl("hBBr:Start init Vulkan Memory Allocator(VMA).");
	InitVMA();
	ConsoleDebug::print_endl("hBBr:Start init Command Pool.");
	CreateCommandPool();
	ConsoleDebug::print_endl("hBBr:Start Create Descripotr Pool.");
	CreateDescripotrPool(_descriptorPool);

	CreateQueryPool(256, _queryTimeStamp);
}

VulkanManager::~VulkanManager()
{
	DestroyQueryPool(_queryTimeStamp);
	DestroyDescriptorPool(_descriptorPool);
	DestroyCommandPool();
	if (_bDebugEnable)
		fvkDestroyDebugReportCallbackEXT(_instance, _debugReport, VK_NULL_HANDLE);
	if (_vma_allocator != VK_NULL_HANDLE)
	{
		vmaDestroyAllocator(_vma_allocator);
		_vma_allocator = VK_NULL_HANDLE;
	}
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
			if (_bDebugEnable)
			{
				if (strcmp(availableLaters[i].layerName, "VK_LAYER_KHRONOS_validation") == 0)
				{
					layers.push_back("VK_LAYER_KHRONOS_validation");
					layerLogs.push_back("hBBr:[Vulkan Instance layer] Add VK_LAYER_KHRONOS_validation layer.");
					bVK_LAYER_KHRONOS_validation = true;
				}
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
		vkEnumerateInstanceExtensionProperties(nullptr, &ecount, VK_NULL_HANDLE);
		std::vector<VkExtensionProperties> availableExts(ecount);
		vkEnumerateInstanceExtensionProperties(nullptr, &ecount, availableExts.data());
		for (uint32_t i = 0; i < ecount; i++)
		{
			ConsoleDebug::print_endl(HString("\t") + availableExts[i].extensionName, "150,150,150");
#ifdef _WIN32
			if (strcmp(availableExts[i].extensionName, VK_KHR_DISPLAY_EXTENSION_NAME) == 0)
			{
				//这个层会自动添加RenderDoc Layer，会报错
				//layers.push_back(availableLaters[i].layerName);
				extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
				_enable_VK_KHR_display = true;
			}
#endif
			if (_bDebugEnable && strcmp(availableExts[i].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				layerLogs.push_back("hBBr:[Vulkan Instance extension] Add VK_EXT_DEBUG_REPORT_EXTENSION_NAME ext.");

			}
			else if (_bDebugEnable && strcmp(availableExts[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				layerLogs.push_back("hBBr:[Vulkan Instance extension] Add VK_EXT_DEBUG_UTILS_EXTENSION_NAME ext.");
			}
			else if (strcmp(availableExts[i].extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
				layerLogs.push_back("hBBr:[Vulkan Instance extension] Add VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME ext.");
			}
			else if (strcmp(availableExts[i].extensionName, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
				layerLogs.push_back("hBBr:[Vulkan Instance extension] Add VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME ext.");
			}
			else if (strcmp(availableExts[i].extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0)
			{
				extensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
				layerLogs.push_back("hBBr:[Vulkan Instance extension] Add VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME ext.");
			}
			else if (strcmp(availableExts[i].extensionName, "VK_KHR_get_surface_capabilities2") == 0)
			{
				extensions.push_back("VK_KHR_get_surface_capabilities2");
				layerLogs.push_back("hBBr:[Vulkan Instance extension] Add VK_KHR_get_surface_capabilities2 ext.");
			}
		}
		ConsoleDebug::print_endl("\t---------End Enumerate Instance Extension Properties------");

		for (auto i : layerLogs)
		{
			ConsoleDebug::print_endl(i);
		}	
	}

	//SDL
	unsigned int eCount = 0;
	SDL_Vulkan_GetInstanceExtensions(&eCount);
	auto sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&eCount);
	for (unsigned int i = 0; i < eCount; i++)
	{
		extensions.push_back(sdl_extensions[i]);
	}
	
	uint32_t apiVersion = VK_API_VERSION_1_3;
    auto GetApiVersionResult = vkEnumerateInstanceVersion(&apiVersion);
	if (VK_SUCCESS != GetApiVersionResult)
	{
		apiVersion = API_VERSION;
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = VK_NULL_HANDLE;
	appInfo.pApplicationName = "hBBr";
	appInfo.pEngineName = "hBBr";
	appInfo.apiVersion = apiVersion;
	appInfo.applicationVersion = 0;
	appInfo.engineVersion = 0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.pApplicationInfo = &appInfo;

	ConsoleDebug::print_endl("hBBr:Enable Vulkan instance layer---------");
	for (auto& i : layers)
	{
		ConsoleDebug::print_endl(i);
	}
	ConsoleDebug::print_endl("hBBr:-------------------------------------");

	ConsoleDebug::print_endl("hBBr:Enable Vulkan instance layer extension---");
	for (auto& i : extensions)
	{
		ConsoleDebug::print_endl(i);
	}
	ConsoleDebug::print_endl("hBBr:-------------------------------------");

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
		MessageOut(GetInternationalizationText("Renderer", "A000000"), false, true);
	}
	else if (result != VK_SUCCESS) {
		MessageOut(GetInternationalizationText("Renderer", "A000001"), false, true);
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
			MessageOut(GetInternationalizationText("Renderer", "A000002"), false, true);
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
			MessageOut(GetInternationalizationText("Renderer", "A000003"), false, true);
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
			if (_bDebugEnable)
			{
				if (strcmp(availableLaters[i].layerName, "VK_LAYER_KHRONOS_validation") == 0 && _bDebugEnable)
				{
					layers.push_back("VK_LAYER_KHRONOS_validation");
					layerLogs.push_back("hBBr:[Vulkan Device layer] Add VK_LAYER_KHRONOS_validation layer.");
				}
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
		vkEnumerateDeviceExtensionProperties(_gpuDevice, nullptr, &ecount, VK_NULL_HANDLE);
		std::vector<VkExtensionProperties> availableExts(ecount);
		vkEnumerateDeviceExtensionProperties(_gpuDevice, nullptr, &ecount, availableExts.data());
		ConsoleDebug::print_endl("\tDevice Extension Properties---------");
		bool bHasRenderPass2Ext = false;
		for (uint32_t i = 0; i < ecount; i++)
		{
			ConsoleDebug::print_endl(HString("\t") + availableExts[i].extensionName, "150,150,150");
			//Debug Marker
			{
				if (_bDebugEnable && strcmp(availableExts[i].extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_debug_marker ext.");
					_deviceExtensionOptionals.HasKHRDebugMarker = 1;
					debugMarkerActive = true;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_SWAPCHAIN_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRSwapchain = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasExtendedDynamicState = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_MAINTENANCE1_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRMaintenance1 = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_MAINTENANCE2_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_MAINTENANCE2_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRMaintenance2 = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_MEMORY_BUDGET_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasMemoryBudget = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasMemoryPriority = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRDedicatedAllocation = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRDedicatedHostOperations = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRShaderFloatControls = 1;
				}
				else if (_deviceExtensionOptionals.HasKHRShaderFloatControls && strcmp(availableExts[i].extensionName, VK_KHR_SPIRV_1_4_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_SPIRV_1_4_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRSpirv_1_4 = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRImageFormatList = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_EXT_VALIDATION_CACHE_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_VALIDATION_CACHE_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRImageFormatList = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_AMD_BUFFER_MARKER_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_AMD_BUFFER_MARKER_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_AMD_BUFFER_MARKER_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasAMDBufferMarker = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRExternalFence = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRExternalSemaphore = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_HUAWEI_SUBPASS_SHADING_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_HUAWEI_SUBPASS_SHADING_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_HUAWEI_SUBPASS_SHADING_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRHuaWeiSubpassShading = 1;
				}
				else if (strcmp(availableExts[i].extensionName, "VK_HUAWEI_smart_cache") == 0)
				{
					extensions.push_back("VK_HUAWEI_smart_cache");
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_HUAWEI_smart_cache ext.");
					_deviceExtensionOptionals.HasKHRHuaWeiSmartCache = 1;
				}
				else if (strcmp(availableExts[i].extensionName, "VK_HUAWEI_prerotation") == 0)
				{
					extensions.push_back("VK_HUAWEI_prerotation");
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_HUAWEI_prerotation ext.");
					_deviceExtensionOptionals.HasKHRHuaWeiSmartCache = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasQcomRenderPassTransform = 1;
				}

				else if (strcmp(availableExts[i].extensionName, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)==0)
				{
					bHasRenderPass2Ext = true;
					extensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRCreateRenderPass2 = 1;
				}
				else if (bHasRenderPass2Ext && strcmp(availableExts[i].extensionName, VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME) == 0)
				{
					//允许深度/模板图像的图像存储屏障仅设置了深度或模板位之一，而不是两者都设置。
					extensions.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasKHRSeparateDepthStencilLayouts = 1;
				} 
				else if (strcmp(availableExts[i].extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0)
				{
					//全屏支持
					extensions.push_back(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);		
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasEXTFullscreenExclusive = 1;
				}
				else if (strcmp(availableExts[i].extensionName, VK_EXT_FILTER_CUBIC_EXTENSION_NAME) == 0)
				{
					extensions.push_back(VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
					layerLogs.push_back("hBBr:[Vulkan Device extension] Add VK_EXT_FILTER_CUBIC_EXTENSION_NAME ext.");
					_deviceExtensionOptionals.HasExtFilter_Cubic = 1;
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
	//禁用所有与稀疏相关的东西
	_gpuFeatures.shaderResourceResidency = VK_FALSE;
	_gpuFeatures.shaderResourceMinLod = VK_FALSE;
	_gpuFeatures.sparseBinding = VK_FALSE;
	_gpuFeatures.sparseResidencyBuffer = VK_FALSE;
	_gpuFeatures.sparseResidencyImage2D = VK_FALSE;
	_gpuFeatures.sparseResidencyImage3D = VK_FALSE;
	_gpuFeatures.sparseResidency2Samples = VK_FALSE;
	_gpuFeatures.sparseResidency4Samples = VK_FALSE;
	_gpuFeatures.sparseResidency8Samples = VK_FALSE;
	_gpuFeatures.sparseResidencyAliased = VK_FALSE;

	////开启vk的gpu特殊功能
	//_gpuVk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	//_gpuVk12Features.separateDepthStencilLayouts = VK_TRUE;
	//_gpuVk12Features.hostQueryReset = VK_TRUE;

	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_info.size());
	device_create_info.pQueueCreateInfos = queue_create_info.data();
	device_create_info.ppEnabledLayerNames = layers.data();
	device_create_info.enabledLayerCount = (uint32_t)layers.size();
	device_create_info.ppEnabledExtensionNames = extensions.data();
	device_create_info.enabledExtensionCount = (uint32_t)extensions.size();
	device_create_info.pEnabledFeatures = &_gpuFeatures;
	//device_create_info.pNext = &_gpuVk12Features;
	auto result = vkCreateDevice(_gpuDevice, &device_create_info, VK_NULL_HANDLE, &_device);
	if(result!= VK_SUCCESS) 
		MessageOut((GetInternationalizationText("Renderer", "A000004")+ GetVkResult(result)) , false, true);
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

void VulkanManager::InitVMA()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = _gpuDevice;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	auto result = vmaCreateAllocator(&allocatorInfo, &_vma_allocator);
	if (result != VK_SUCCESS)
	{
		MessageOut(GetInternationalizationText("Renderer", "A000029"), true, true);
	}
}

void VulkanManager::CreateQueryPool(uint32_t queryCount, VkQueryPool& poolInOut, VkQueryType type)
{
	if (poolInOut)
	{
		DestroyQueryPool(poolInOut);
	}
	VkQueryPoolCreateInfo queryPoolInfo{};
	queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queryPoolInfo.queryType = type;
	queryPoolInfo.queryCount = queryCount;
	vkCreateQueryPool(_device, &queryPoolInfo, nullptr, &poolInOut);

}

void VulkanManager::DestroyQueryPool(VkQueryPool& poolInOut)
{
	if (poolInOut)
	{
		vkDestroyQueryPool(_device, poolInOut, nullptr);
	}
	poolInOut = nullptr;
}

uint32_t VulkanManager::FindMemoryTypeIndex(const VkMemoryRequirements* memory_requirements, const VkMemoryPropertyFlags required_properties)
{
	//for (uint32_t i = 0; i < _gpuMemoryProperties.memoryTypeCount; ++i) {
	//	if (memory_requirements->memoryTypeBits & (1 << i)) {
	//		if ((_gpuMemoryProperties.memoryTypes[i].propertyFlags & required_properties) == required_properties) {
	//			return i;
	//		}
	//	}
	//}
	//MessageOut("Cound not find memory type.");
	//return UINT32_MAX;

	for (uint32_t i = 0; i < _gpuMemoryProperties.memoryTypeCount; i++)
	{
		if ((_gpuMemoryProperties.memoryTypes[i].propertyFlags & required_properties) == required_properties && memory_requirements->memoryTypeBits & (1 << i))
		{
			return i;
		}
	}
	return 0xFFFFFFFF; // Unable to find memoryType

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

void VulkanManager::ReCreateSurface_SDL(SDL_Window* handle, VkSurfaceKHR& newSurface)
{
	if (newSurface != VK_NULL_HANDLE)
	{
		DestroySurface(newSurface);
		newSurface = VK_NULL_HANDLE;
	}
	//SDL2
	auto result = SDL_Vulkan_CreateSurface(handle, _instance, nullptr, &newSurface);
	if (result != 0)
	{
		MessageOut("sdl Create Window Surface Failed.", false, true);
	}
}

void VulkanManager::DestroySurface(VkSurfaceKHR& surface)
{
	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(_instance, surface, VK_NULL_HANDLE);
		surface = VK_NULL_HANDLE;
	}
}

void VulkanManager::GetSurfaceSize(VkSurfaceKHR surface, VkExtent2D& surfaceSize)
{
	const int SwapchainBufferCount = 3;
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
}

void VulkanManager::DeviceWaitIdle()
{
	vkDeviceWaitIdle(_device);
}

void VulkanManager::QueueWaitIdle(VkQueue queue)
{
	vkQueueWaitIdle(queue);
}

void VulkanManager::CheckSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR& surfaceFormat)
{
	//Check Support
	VkBool32 IsSupportSurface = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(_gpuDevice, _graphicsQueueFamilyIndex, surface, &IsSupportSurface);
	if (!IsSupportSurface)
	{
		MessageOut(GetInternationalizationText("Renderer", "A000006"), false, true);
	}
	{
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_R8G8B8A8_UNORM , VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		uint32_t avail_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_gpuDevice, surface, &avail_count, nullptr);
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

bool VulkanManager::GetSurfaceCapabilities(VkSurfaceKHR& surface, VkSurfaceCapabilitiesKHR* surfaceCapabilities)
{
	auto vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpuDevice, surface, surfaceCapabilities);
	if (vkResult != VK_SUCCESS)
	{
		ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000024"));
		return false;
	}
	return true;
}

VkExtent2D VulkanManager::CreateSwapchain(
	SDL_Window* window,
	VkExtent2D surfaceSize, 
	VkSurfaceKHR surface, 
	VkSurfaceFormatKHR surfaceFormat , 
	VkSwapchainKHR &newSwapchain, 
	std::vector<VkImage>& swapchainImages, 
	std::vector<VkImageView>& swapchainImageViews, 
	VkSurfaceCapabilitiesKHR& surfaceCapabilities,
	bool bIsFullScreen,
	bool bVSync
)
{
	//ConsoleDebug::print_endl("Create Swapchain KHR.");
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	//Vulkan的垂直同步
#if 0
	//if (_winInfo.vsync)
	{
		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, VK_NULL_HANDLE);
		if (present_mode_count <= 0)
		{
			MessageOut("Vulkan Error:Can not Find any VkPresentModeKHR!", false,true,"255,0,0");
			_Sleep(50);
			return CreateSwapchain(surfaceSize, surface, surfaceFormat, newSwapchain,
				swapchainImages, swapchainImageViews, surfaceCapabilities, cmdBuf, acquireImageSemaphore, queueSubmitSemaphore, fences, bIsFullScreen);
		}
		std::vector<VkPresentModeKHR> presentModes(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuDevice, surface, &present_mode_count, presentModes.data());

		bool bFoundPresentModeMailbox = false;
		bool bFoundPresentModeImmediate = false;
		bool bFoundPresentModeFIFO = false;
		bool bFoundPresentModeFIFORelaxed = false;
		bool bFoundPresentModeSharedDemandRefresh = false;
		bool bFoundPresentModeSharedContinuous = false;
		HString CurrentPresentMode = "";
		for (size_t i = 0; i < present_mode_count; i++)
		{
			switch (presentModes[i])
			{
			case VK_PRESENT_MODE_MAILBOX_KHR:
				bFoundPresentModeMailbox = true;
				//ConsoleDebug::printf_endl("- VK_PRESENT_MODE_MAILBOX_KHR : (%d)",(int)VK_PRESENT_MODE_MAILBOX_KHR);
				break;
			case VK_PRESENT_MODE_IMMEDIATE_KHR:
				bFoundPresentModeImmediate = true;
				//ConsoleDebug::printf_endl("- VK_PRESENT_MODE_IMMEDIATE_KHR : (%d)", (int)VK_PRESENT_MODE_IMMEDIATE_KHR);
				break;
			case VK_PRESENT_MODE_FIFO_KHR:
				bFoundPresentModeFIFO = true;
				//ConsoleDebug::printf_endl("- VK_PRESENT_MODE_FIFO_KHR : (%d)", (int)VK_PRESENT_MODE_FIFO_KHR);
				break;
			case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
				//ConsoleDebug::printf_endl("- VK_PRESENT_MODE_FIFO_RELAXED_KHR : (%d)", (int)VK_PRESENT_MODE_FIFO_RELAXED_KHR);
				bFoundPresentModeFIFORelaxed = true;
				break;
			case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
				//ConsoleDebug::printf_endl("- VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR : (%d)", (int)VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
				bFoundPresentModeSharedDemandRefresh = true;
				break;
			case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
				//ConsoleDebug::printf_endl("- VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR : (%d)", (int)VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);
				bFoundPresentModeSharedContinuous = true;
				break;
			default:
				ConsoleDebug::printf_endl("- Other VkPresentModeKHR : (%d)", (int)presentModes[i]);
				break;
			}
		}

		int RequestedPresentMode = -1;
		{
			bool bRequestSuccessful = false;
			switch (RequestedPresentMode)
			{
			case VK_PRESENT_MODE_IMMEDIATE_KHR:
				if (bFoundPresentModeImmediate)
				{
					present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
					CurrentPresentMode = "VK_PRESENT_MODE_IMMEDIATE_KHR";
					bRequestSuccessful = true;
				}
				break;
			case VK_PRESENT_MODE_MAILBOX_KHR:
				if (bFoundPresentModeMailbox)
				{
					present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
					CurrentPresentMode = "VK_PRESENT_MODE_MAILBOX_KHR";
					bRequestSuccessful = true;
				}
				break;
			case VK_PRESENT_MODE_FIFO_KHR:
				if (bFoundPresentModeFIFO)
				{
					present_mode = VK_PRESENT_MODE_FIFO_KHR;
					CurrentPresentMode = "VK_PRESENT_MODE_FIFO_KHR";
					bRequestSuccessful = true;
				}
				break;
			case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
				if (bFoundPresentModeFIFORelaxed)
				{
					present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
					CurrentPresentMode = "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
					bRequestSuccessful = true;
				}
				break;
			default:
				break;
			}

			if (!bRequestSuccessful)
			{
				ConsoleDebug::printf_endl_warning("Requested PresentMode (%d) is not handled or available, ignoring...", RequestedPresentMode);
				RequestedPresentMode = -1;
			}
		}

		if (RequestedPresentMode == -1)
		{
			// Until FVulkanViewport::Present honors SyncInterval, we need to disable vsync for the spectator window if using an HMD.
			if ( !bVSync)
			{
				if (bFoundPresentModeImmediate)
				{
					present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;//类似双缓冲
					CurrentPresentMode = "VK_PRESENT_MODE_IMMEDIATE_KHR";
				}
				else if (bFoundPresentModeMailbox)
				{
					present_mode = VK_PRESENT_MODE_MAILBOX_KHR;//类似三缓冲
					CurrentPresentMode = "VK_PRESENT_MODE_MAILBOX_KHR";
				}
			}
			else if (bFoundPresentModeFIFO)
			{
				present_mode = VK_PRESENT_MODE_FIFO_KHR;//垂直同步
				CurrentPresentMode = "VK_PRESENT_MODE_FIFO_KHR";
			}
			else if (bFoundPresentModeFIFORelaxed)
			{
				present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;//垂直同步
				CurrentPresentMode = "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
			}
			else
			{
				ConsoleDebug::printf_endl_error("Couldn't find desired PresentMode! Using %d", presentModes[0]);
				present_mode = presentModes[0];
			}
		}
		ConsoleDebug::printf_endl("Current presene mode is : " + CurrentPresentMode + " :%d ", (int)VK_PRESENT_MODE_MAILBOX_KHR);
	}
#else

	//VK_PRESENT_MODE_IMMEDIATE_KHR：
			// 立即模式。在图像准备好后立即显示，可能会导致撕裂现象。这种模式下，GPU渲染的速度不受显示器刷新率的限制。
	//VK_PRESENT_MODE_MAILBOX_KHR：
			// 邮箱模式。类似于立即模式，但当新图像准备好时，它会替换交换链中的旧图像，而不是立即显示。这可以减少撕裂现象，但仍然允许GPU渲染的速度超过显示器的刷新率。
	//VK_PRESENT_MODE_FIFO_KHR：
			// 先进先出（FIFO）模式。在显示器完成刷新后，才从交换链中获取新图像并显示。这种模式下，GPU渲染的速度受到显示器刷新率的限制，但可以防止撕裂现象。这是Vulkan API规范中要求必须支持的呈现模式。
	//VK_PRESENT_MODE_FIFO_RELAXED_KHR：
			// 宽松的FIFO模式。当图像准备好并且显示器处于垂直同步间隔之外时，立即显示图像。这种模式下，当GPU渲染速度低于显示器刷新率时，可以避免撕裂现象；当GPU渲染速度高于显示器刷新率时，可能会出现撕裂现象。
	present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
#endif

	VkSurfaceTransformFlagBitsKHR PreTransform;
	//if(surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	//{
	//	PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	//}
	//else
	{
		PreTransform = surfaceCapabilities.currentTransform;
	}

	switch (PreTransform)
	{
	case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:
		//ConsoleDebug::print_endl("Current preTransform is : VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR");
		break;
	case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
		//ConsoleDebug::print_endl("Current preTransform is : VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR");
		break;
	case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
		//ConsoleDebug::print_endl("Current preTransform is : VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR");
		break;
	case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
		//ConsoleDebug::print_endl("Current preTransform is : VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR");
		break;
	default:break;
	}

	VkCompositeAlphaFlagBitsKHR CompositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	{
		CompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}

	// 0 means no limit, so use the requested number
	uint32_t DesiredNumBuffers = surfaceCapabilities.maxImageCount > 0 ? std::clamp(_swapchainBufferCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount) : _swapchainBufferCount;

	//uint32_t SizeX = true ? (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF ? surfaceSize.width : surfaceCapabilities.currentExtent.width) : surfaceSize.width;
	//uint32_t SizeY = true ? (surfaceCapabilities.currentExtent.height == 0xFFFFFFFF ? surfaceSize.height : surfaceCapabilities.currentExtent.height) : surfaceSize.height;

	if (surfaceCapabilities.currentExtent.width < UINT32_MAX && surfaceCapabilities.currentExtent.width>0) {
		surfaceSize.width = surfaceCapabilities.currentExtent.width;
		surfaceSize.height = surfaceCapabilities.currentExtent.height;
	}
	else {
		surfaceSize.width = surfaceCapabilities.maxImageExtent.width < (uint32_t)surfaceSize.width ? surfaceCapabilities.maxImageExtent.width : (uint32_t)surfaceSize.width;
		surfaceSize.width = surfaceCapabilities.minImageExtent.width > (uint32_t)surfaceSize.width ? surfaceCapabilities.minImageExtent.width : (uint32_t)surfaceSize.width;
		surfaceSize.height = surfaceCapabilities.maxImageExtent.height < (uint32_t)surfaceSize.height ? surfaceCapabilities.maxImageExtent.height : (uint32_t)surfaceSize.height;
		surfaceSize.height = surfaceCapabilities.minImageExtent.height > (uint32_t)surfaceSize.height ? surfaceCapabilities.minImageExtent.height : (uint32_t)surfaceSize.height;
	}

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.imageFormat = surfaceFormat.format;
	info.imageColorSpace = surfaceFormat.colorSpace;
	info.minImageCount = DesiredNumBuffers;
	//info.imageExtent.width = SizeX;
	//info.imageExtent.height = SizeY;
	info.imageExtent = surfaceSize;
	info.imageUsage =
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT	//支持在RenderPass中作为color附件，并且在subpass中进行传递
		| VK_IMAGE_USAGE_TRANSFER_SRC_BIT				//支持复制到其他图像
		| VK_IMAGE_USAGE_TRANSFER_DST_BIT				//支持从其他数据复制进来
		| VK_IMAGE_USAGE_SAMPLED_BIT						//支持被采样
		;
	info.preTransform = PreTransform;
	info.imageArrayLayers = 1;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.presentMode = present_mode;
	info.oldSwapchain = VK_NULL_HANDLE;
	info.clipped = VK_TRUE;
	info.compositeAlpha = CompositeAlpha;//是否半透明，用于组合其他表面,这里我们不需要

	{
		//#todo-rco: Crappy workaround
		if (info.imageExtent.width == 0)
		{
			info.imageExtent.width = surfaceSize.width;
		}
		if (info.imageExtent.height == 0)
		{
			info.imageExtent.height = surfaceSize.height;
		}
	}

	if (surfaceCapabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR ||
		surfaceCapabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
	{
		// Swap to get identity width and height
		std::swap(info.imageExtent.width, info.imageExtent.height);
	}

	VkSurfaceFullScreenExclusiveInfoEXT FullScreenInfo = {};
	#ifdef _WIN32
	VkSurfaceFullScreenExclusiveWin32InfoEXT fullScreenExclusiveWin32Info = {};
	#endif
	if (_deviceExtensionOptionals.HasEXTFullscreenExclusive)
	{
		#ifdef _WIN32
			//fullscreen support
			HWND hWnd = (HWND)VulkanApp::GetWindowHandle(window);
			//dwFlags：一个 DWORD 类型的值，表示如何选择与窗口关联的显示器。可能的值包括：
			//MONITOR_DEFAULTTONULL：如果窗口没有与显示器重叠，则返回 NULL。
			//MONITOR_DEFAULTTOPRIMARY：如果窗口没有与显示器重叠，则返回主显示器。
			//MONITOR_DEFAULTTONEAREST：如果窗口没有与显示器重叠，则返回最接近窗口的显示器。
			HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
			fullScreenExclusiveWin32Info.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT;
			fullScreenExclusiveWin32Info.hmonitor = hMonitor;

			FullScreenInfo.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT;
			FullScreenInfo.fullScreenExclusive = bIsFullScreen ? VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT : VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT;
			FullScreenInfo.pNext = &fullScreenExclusiveWin32Info;
			info.pNext = &FullScreenInfo;
		#endif
	}
	auto result = vkCreateSwapchainKHR(_device, &info, VK_NULL_HANDLE, &newSwapchain);
	if (result != VK_SUCCESS)
	{
#ifdef _WIN32
		//全屏可能失败了,取消全屏
		if (result == VK_ERROR_INITIALIZATION_FAILED)
		{
			//ConsoleDebug::printf_endl_warning("vkCreateSwapchainKHR return VK_ERROR_INITIALIZATION_FAILED . Create swapchain failed with Initialization error; removing FullScreen extension...");
			info.pNext = nullptr;
			result = vkCreateSwapchainKHR(_device, &info, nullptr, &newSwapchain);
		}
        if (result != VK_SUCCESS)
#endif
		{
			MessageOut((GetInternationalizationText("Renderer", "A000007") + GetVkResult(result)), false, true);
		}
	}

	_swapchainBufferCount = DesiredNumBuffers;

	//获取交换链Image,注意数量可能与_swapchainBufferCount不一致
	uint32_t numSwapchainImages = surfaceCapabilities.minImageCount + 1;
	vkGetSwapchainImagesKHR(_device, newSwapchain, &numSwapchainImages, nullptr);
	swapchainImages.resize(numSwapchainImages);
	vkGetSwapchainImagesKHR(_device, newSwapchain, &numSwapchainImages, swapchainImages.data());

	//创建ImageView
	//ConsoleDebug::print_endl("hBBr:Swapchain: Create Swapchain Image View.");
	swapchainImageViews.resize(numSwapchainImages);
	for (int i = 0; i < (int)numSwapchainImages; i++)
	{
		CreateImageView(swapchainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, swapchainImageViews[i]);
	}
	//ConsoleDebug::print_endl("hBBr:Swapchain: Transition Swapchain Image layout to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.");
	//Swapchain转换到呈现布局
	VkCommandBuffer buf;
	AllocateCommandBuffer(_commandPool, buf);
	BeginCommandBuffer(buf, 0);
	for (int i = 0; i < (int)numSwapchainImages; i++)
	{
		Transition(buf, swapchainImages[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
	EndCommandBuffer(buf);
	SubmitQueueImmediate({buf});
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	FreeCommandBuffer(_commandPool, buf);

	_swapchainBufferCount = numSwapchainImages;

	return info.imageExtent;
}

void VulkanManager::DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<VkImageView>& swapchainImageViews)
{
	for (auto& i: swapchainImageViews)
	{
		vkDestroyImageView(_device, i, VK_NULL_HANDLE);
		i = VK_NULL_HANDLE;
	}
	swapchainImageViews.clear();
	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_device, swapchain, VK_NULL_HANDLE);
		swapchain = VK_NULL_HANDLE;
	}
}

void VulkanManager::CreateImage(uint32_t width , uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, VkImage& image, uint32_t miplevel, uint32_t layerCount)
{
	VkExtent2D texSize = {};
	texSize.width = width;
	texSize.height = height;
	VkImageCreateInfo	create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.flags = 0;
	if (layerCount == 6)
	{
		create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}
	create_info.format = format;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.usage = usageFlags;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.arrayLayers = layerCount;
	create_info.mipLevels = miplevel;
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

void VulkanManager::CreateImageView(VkImage inImage, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView, uint32_t miplevel, uint32_t layerCount, VkComponentMapping componentMapping)
{
	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.flags = 0;
	image_view_create_info.image = inImage;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	if (layerCount == 6)
	{
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	}
	image_view_create_info.format = format;
	image_view_create_info.components = componentMapping;
	image_view_create_info.subresourceRange.aspectMask = aspectFlags;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.layerCount = layerCount;
	image_view_create_info.subresourceRange.levelCount = miplevel;
	vkCreateImageView(_device, &image_view_create_info, VK_NULL_HANDLE, &imageView);
}

VkDeviceSize VulkanManager::CreateImageMemory(VkImage inImage, VkDeviceMemory& imageViewMemory, VkMemoryPropertyFlags memoryPropertyFlag)
{
	if (inImage == VK_NULL_HANDLE)
	{
		MessageOut("Create vulkan image view failed.VkImage is nullptr.", false, false);
		return 0 ;
	}
	VkMemoryRequirements mem_requirement;
	vkGetImageMemoryRequirements(_device, inImage, &mem_requirement);
	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = mem_requirement.size;
	memory_allocate_info.memoryTypeIndex = FindMemoryTypeIndex(&mem_requirement, memoryPropertyFlag);
	auto err = vkAllocateMemory(_device, &memory_allocate_info, VK_NULL_HANDLE, &imageViewMemory);
	if (VK_SUCCESS != err) {
		MessageOut("Create vulkan image view failed.VkImage is nullptr.", false, false);
	}
	err = vkBindImageMemory(_device, inImage, imageViewMemory, 0);
	if (VK_SUCCESS != err) {
		MessageOut("Create vulkan image view failed.VkImage is nullptr.", false, false);
	}
	if (VK_SUCCESS == err) {
		return mem_requirement.size;
	}
	return 0;
}

bool VulkanManager::CheckImageProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usages, VkImageCreateFlags flags, VkImageFormatProperties* out)
{
	bool result = true;
	VkImageFormatProperties formatProperties = {};
	vkGetPhysicalDeviceImageFormatProperties(
		_gpuDevice,
		format,
		type, tiling,
		usages,
		flags,
		&formatProperties);
	if (
		formatProperties.maxArrayLayers <= 0
		&& formatProperties.maxExtent.width <= 0
		&& formatProperties.maxExtent.height <= 0
		&& formatProperties.maxExtent.depth <= 0
		&& formatProperties.maxMipLevels <= 0
		&& formatProperties.maxResourceSize <= 0
		&& formatProperties.sampleCounts <= 0
		)
	{
		result = false;
	}
	if (out != nullptr)
	{
		*out = formatProperties;
	}
	return result;
}

void VulkanManager::Transition(
	VkCommandBuffer cmdBuffer, 
	VkImage image, 
	VkImageAspectFlags aspects, 
	VkImageLayout oldLayout, 
	VkImageLayout newLayout, 
	uint32_t mipLevelBegin , 
	uint32_t mipLevelCount, 
	uint32_t baseArrayLayer, 
	uint32_t layerCount)
{
	if (oldLayout == newLayout)
	{
		return;
	}
	VkImageMemoryBarrier imageBarrier = {};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.pNext = nullptr;
	imageBarrier.oldLayout = oldLayout;
	imageBarrier.newLayout = newLayout;
	imageBarrier.image = image;
	imageBarrier.subresourceRange.aspectMask = aspects;
	imageBarrier.subresourceRange.baseMipLevel = mipLevelBegin;
	imageBarrier.subresourceRange.levelCount = mipLevelCount;
	imageBarrier.subresourceRange.baseArrayLayer = baseArrayLayer;
	imageBarrier.subresourceRange.layerCount = layerCount;
	VkPipelineStageFlags srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	if (
		(oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL || oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		)
	{
		imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcFlags = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dstFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		switch (oldLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			imageBarrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			imageBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			srcFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
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
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		default:
			break;
		}

		switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dstFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dstFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			imageBarrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dstFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dstFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			break;
		default:
			break;
		}
	}
	
	vkCmdPipelineBarrier(cmdBuffer, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1,
		&imageBarrier);
}

void VulkanManager::DestroyImage(VkImage& image)
{
	if (image != VK_NULL_HANDLE)
	{
		vkDestroyImage(_device, image, VK_NULL_HANDLE);
	}
}

void VulkanManager::DestroyImageView(VkImageView& imageView)
{
	if (imageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(_device, imageView, VK_NULL_HANDLE);
	}
}

void VulkanManager::CreateSampler(VkSampler& sampler, VkFilter filter, VkSamplerAddressMode address, float minMipLeve, float maxMipLevel)
{
	VkSamplerCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.magFilter = filter;
	info.minFilter = filter;
	if (filter == VK_FILTER_NEAREST)
	{
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}
	else
	{
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
	info.addressModeU = address;
	info.addressModeV = address;
	info.addressModeW = address;
	//各项异性采样
	if (_gpuFeatures.samplerAnisotropy == VK_TRUE)
	{
		info.anisotropyEnable = VK_TRUE;
		info.maxAnisotropy = 16.0f;
	}
	else
	{
		info.anisotropyEnable = VK_FALSE;
		info.maxAnisotropy = 1.0f;
	}
	info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	info.unnormalizedCoordinates = VK_FALSE;
	info.compareEnable = VK_FALSE;
	info.compareOp = VK_COMPARE_OP_ALWAYS;
	info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	info.minLod = minMipLeve;
	info.maxLod = maxMipLevel;
	vkCreateSampler(_device, &info, nullptr, &sampler);
}

void VulkanManager::CreateCommandPool()
{
	CreateCommandPool(_commandPool);
}

void VulkanManager::ResetCommandPool()
{
	ResetCommandPool(_commandPool);
}

void VulkanManager::DestroyCommandPool()
{
	DestroyCommandPool(_commandPool);
}

void VulkanManager::CreateCommandPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = nullptr;
	cmdPoolInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	cmdPoolInfo.flags = flags;

	vkCreateCommandPool(_device, &cmdPoolInfo, VK_NULL_HANDLE, &commandPool);
}

void VulkanManager::ResetCommandPool(VkCommandPool& commandPool)
{
	if (commandPool != VK_NULL_HANDLE)
	{
		vkResetCommandPool(_device, commandPool, 0);
	}
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
	cmdBufAllocInfo.pNext = nullptr;
	cmdBufAllocInfo.commandPool = commandPool;
	cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocInfo.commandBufferCount = 1;
	VkResult result = vkAllocateCommandBuffers(_device, &cmdBufAllocInfo,&cmdBuf);
	if (result != VK_SUCCESS)
	{
		MessageOut(GetInternationalizationText("Renderer", "A000008"), true, true);
	}
}

void VulkanManager::FreeCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer>& cmdBufs)
{
	if (cmdBufs.size() > 0)
	{
		vkFreeCommandBuffers(_device, commandPool, (uint32_t)cmdBufs.size(), cmdBufs.data());
		cmdBufs.clear();
	}
}

void VulkanManager::FreeCommandBuffer(VkCommandPool commandPool, VkCommandBuffer& cmdBuf)
{
	if (cmdBuf != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(_device, commandPool, 1, &cmdBuf);
	}
	cmdBuf = VK_NULL_HANDLE;
}

void VulkanManager::ResetCommandBuffer(VkCommandBuffer& cmdBuf)
{
	vkResetCommandBuffer(cmdBuf, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void VulkanManager::BeginCommandBuffer(VkCommandBuffer& cmdBuf, VkCommandBufferUsageFlags flag)
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

void VulkanManager::EndCommandBuffer(VkCommandBuffer& cmdBuf)
{
	vkEndCommandBuffer(cmdBuf);
}

void VulkanManager::BeginRenderPass(VkCommandBuffer& cmdBuf, VkFramebuffer framebuffer, VkRenderPass renderPass, VkExtent2D areaSize, std::vector<VkAttachmentDescription>_attachmentDescs, std::array<float, 4> clearColor)
{
	const auto clearValueCount = _attachmentDescs.size();

	VkRenderPassBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.framebuffer = framebuffer;
	info.renderPass = renderPass;
	info.renderArea.offset = { 0, 0 };
	info.renderArea.extent = areaSize;
	info.clearValueCount = (uint32_t)clearValueCount;
	std::vector < VkClearValue > clearValues;
	clearValues.reserve(clearValueCount);
	for (int i = 0; i < (int)clearValueCount; i++)
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

void VulkanManager::EndRenderPass(VkCommandBuffer& cmdBuf)
{
	vkCmdEndRenderPass(cmdBuf);
}

bool VulkanManager::GetNextSwapchainIndex(VkSwapchainKHR& swapchain, VkSemaphore& semaphore, VkFence* fence, uint32_t* swapchainIndex)
{
	VkResult result = VK_SUCCESS;
	if (fence == nullptr)
	{
		result = vkAcquireNextImageKHR(_device, swapchain, UINT64_MAX, semaphore, nullptr, swapchainIndex);
	}
	else
	{
		result = vkAcquireNextImageKHR(_device, swapchain, UINT64_MAX, semaphore, *fence, swapchainIndex);
	}
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		#if _DEBUG
		//MessageOut(RendererLauguage::GetText("A000009"), false, false);//太烦人了,不影响
		#endif	
		return false;
	}
	else if (result != VK_SUCCESS)
	{
		MessageOut(GetInternationalizationText("Renderer", "A000010"), false, false);
		return false;
	}
	return true;
}

bool VulkanManager::Present(VkSwapchainKHR& swapchain, VkSemaphore& semaphore, uint32_t& swapchainImageIndex)
{
	VkResult infoResult = VK_SUCCESS;
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &swapchainImageIndex;
	presentInfo.pResults = &infoResult; // Optional
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphore;
	auto result = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		//MessageOut("VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR.Swapchain need to reset.", false, false);//太烦人了,不影响
		return false;
	}
	else if (result != VK_SUCCESS || infoResult != VK_SUCCESS)
	{
		MessageOut(GetInternationalizationText("Renderer", "A000011"), false, true);
		return false;
	}
	return true;
}

bool VulkanManager::Present(VkSwapchainKHR& swapchain, std::vector<VkSemaphore> semaphores, uint32_t& swapchainImageIndex)
{
	VkResult infoResult = VK_SUCCESS;
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &swapchainImageIndex;
	presentInfo.pResults = &infoResult; // Optional
	presentInfo.waitSemaphoreCount = (uint32_t)semaphores.size();
	presentInfo.pWaitSemaphores = semaphores.data();
	auto result = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		//MessageOut("VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR.Swapchain need to reset.", false, false);//太烦人了,不影响
		return false;
	}
	else if (result != VK_SUCCESS || infoResult != VK_SUCCESS)
	{
		MessageOut(GetInternationalizationText("Renderer", "A000011"), false, true);
		return false;
	}
	return true;
}

void VulkanManager::ReCreatePipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayout, VkPipelineLayout& pipelineLayout)
{
	if (pipelineLayout != nullptr)
	{
		DestroyPipelineLayout(pipelineLayout);
		pipelineLayout = nullptr;
	}

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
		MessageOut("vkCreatePipelineLayout error!", true, true);
	}
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
		//{ VK_DESCRIPTOR_TYPE_SAMPLER, 20 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2000 },	//这是一个image和sampler的组合descriptor
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },				//纯image,不带sampler
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
		//{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		//{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.maxSets = 10000;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)std::size(pool_sizes);
	descriptorPoolCreateInfo.pPoolSizes = pool_sizes;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	vkCreateDescriptorPool(_device, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &pool);
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

void VulkanManager::CreateDescripotrSetLayout(std::vector<VkDescriptorType> types, std::vector<VkShaderStageFlags> shaderStageFlags, VkDescriptorSetLayout& descriptorSetLayout)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(types.size());
	for (uint32_t i = 0; i < types.size(); i++)
	{
		bindings[i] = {};
		bindings[i].descriptorType = types[i];
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = shaderStageFlags[i];
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

void VulkanManager::DestroyDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout)
{
	if (descriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(_device, descriptorSetLayout, VK_NULL_HANDLE);
		descriptorSetLayout = VK_NULL_HANDLE;
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
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.flags = 0;
	framebufferCreateInfo.pNext = 0;
	framebufferCreateInfo.layers = 1;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.width = width;
	framebufferCreateInfo.height = height;
	framebufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
	framebufferCreateInfo.pAttachments = attachments.data();
	auto result = vkCreateFramebuffer(_device, &framebufferCreateInfo, VK_NULL_HANDLE, &framebuffer);
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

void VulkanManager::AllocateDescriptorSets(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VkDescriptorSet>& descriptorSet)
{
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = pool;
	info.descriptorSetCount = (uint32_t)descriptorSet.size();
	info.pSetLayouts = &descriptorSetLayout;
	auto result = vkAllocateDescriptorSets(_device, &info, descriptorSet.data());
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

void VulkanManager::FreeDescriptorSet(VkDescriptorPool pool, VkDescriptorSet& descriptorSet)
{
	if (descriptorSet != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets(_device, pool, 1, &descriptorSet);
		descriptorSet = VK_NULL_HANDLE;
	}
}

void VulkanManager::CreateVkSemaphore(VkSemaphore& semaphore)
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = VK_NULL_HANDLE;
	auto result = vkCreateSemaphore(_device, &semaphoreCreateInfo, VK_NULL_HANDLE, &semaphore);
	if (result != VK_SUCCESS)
	{
		MessageOut((HString("Vulkan ERROR: Create Semaphore Failed : ")+ GetVkResult(result)), false, true);
	}
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
		this->CreateVkSemaphore(semaphore[i]);
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

void VulkanManager::CreateFence(VkFence& fence, VkFenceCreateFlags createFlags)
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.flags = createFlags;//default signaled
	VkResult result = vkCreateFence(_device, &info, VK_NULL_HANDLE, &fence);
	if (result != VK_SUCCESS) {
		MessageOut("Create Fence Failed.", false, true);
	}
}

void VulkanManager::RecreateFences(std::vector<VkFence>& fences, uint32_t number)
{
	for (int i = 0; i < fences.size(); i++)
	{
		if (fences[i] != VK_NULL_HANDLE)
		{
			DestroyFence(fences[i]);
		}
	}
	fences.resize(_swapchainBufferCount);
	for (int i = 0; i < (int)_swapchainBufferCount; i++)
	{
		CreateFence(fences[i]);
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

void VulkanManager::ResetFence(VkFence& fence)
{
	vkResetFences(_device, 1, &fence);
}

bool VulkanManager::IsFenceFinish(VkFence& fence)
{
	auto result = vkGetFenceStatus(_device, fence);
	if (result == VK_SUCCESS)
	{
		return true;
	}
	return false;
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
//	//不具备实际数据作用,如果没有查询的意义,随便给一个不同的值就行了,但不能为nullptr(0)
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
		MessageOut(GetInternationalizationText("Renderer", "A000012"), false, true);
	}
}

void VulkanManager::CreateComputePipeline(VkComputePipelineCreateInfo& info, VkPipeline& pipeline)
{
	auto result = vkCreateComputePipelines(_device, nullptr, 1, &info, nullptr, &pipeline);
	if (result != VK_SUCCESS)
	{
		MessageOut(GetInternationalizationText("Renderer", "A000012"), false, true);
	}
}

void VulkanManager::DestroyPipeline(VkPipeline& pipeline)
{
	if (pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(_device, pipeline, VK_NULL_HANDLE);
		pipeline = VK_NULL_HANDLE;
	}
}

void VulkanManager::CreateRenderPass(std::vector<VkAttachmentDescription>attachmentDescs, std::vector<VkSubpassDependency>subpassDependencys, std::vector<VkSubpassDescription>subpassDescs, VkRenderPass& renderPass)
{
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescs.size();
	renderPassCreateInfo.pAttachments = attachmentDescs.data();
	renderPassCreateInfo.dependencyCount = (uint32_t)subpassDependencys.size();
	renderPassCreateInfo.pDependencies = subpassDependencys.data();
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.subpassCount = (uint32_t)subpassDescs.size();
	renderPassCreateInfo.pSubpasses = subpassDescs.data();
	vkCreateRenderPass(_device, &renderPassCreateInfo, VK_NULL_HANDLE, &renderPass);
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
		MessageOut("[ Create Buffer ] Create Uniform Buffer Failed.", false, true, "255,2,0");
	}
}

void VulkanManager::DestroyBuffer(VkBuffer& buffer)
{
	if (buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(_device, buffer, VK_NULL_HANDLE);
	}
	buffer = VK_NULL_HANDLE;
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
		MessageOut("[ Create Buffer ] Allocate Memory Failed! ", false, true, "255,2,0");
	}
	if (vkBindBufferMemory(_device, buffer, bufferMemory, 0) != VK_SUCCESS) {
		MessageOut("[ Create Buffer ] Bind Buffer Memory!  ", false, true, "255,2,0");
	}
}

void VulkanManager::CreateBufferAndAllocateMemory(size_t bufferSize, uint32_t bufferUsage, uint32_t bufferMemoryProperty, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	CreateBuffer(bufferUsage, bufferSize, buffer);
	AllocateBufferMemory(buffer, bufferMemory, bufferMemoryProperty);
}

void VulkanManager::VMACraeteBufferAndAllocateMemory(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo* vmaInfo, VmaMemoryUsage memoryUsage, bool bAlwayMapping, bool bFocusCreateDedicatedMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
	bufferInfo.usage = bufferUsage;
	bufferInfo.size = bufferSize;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = memoryUsage;
	if (bFocusCreateDedicatedMemory)
	{
		//正常情况下VMA会从一块大的内存里分割一部分出来创建Buffer
		//不过也有部分情况，VMA会自动帮我们做这个决定
		//这个flag表示是否强制分配独立内存
		allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	}
	if (bAlwayMapping)
	{
		//永远开启mapping
		allocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
	}

	vmaCreateBuffer(_vma_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, vmaInfo);
}

void VulkanManager::VMADestroyBufferAndFreeMemory(VkBuffer& buffer, VmaAllocation& allocation, HString debugName, VkDeviceSize debugSize)
{
	if (buffer != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(_vma_allocator, buffer, allocation);
		#if IS_EDITOR
		ConsoleDebug::printf_endl(GetInternationalizationText("Renderer", "DestroyBuffer"), debugName.c_str(), debugSize, (double)debugSize / (double)1024.0 / (double)1024.0);
		#endif
	}
	buffer = VK_NULL_HANDLE;
	allocation = VK_NULL_HANDLE;
}

void VulkanManager::DestroyBufferAndMemory(VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	FreeBufferMemory(bufferMemory);
	DestroyBuffer(buffer);
}

void VulkanManager::FreeBufferMemory(VkDeviceMemory& bufferMemory)
{
	if (bufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(_device, bufferMemory, VK_NULL_HANDLE);
	}
	bufferMemory = VK_NULL_HANDLE;
}

bool VulkanManager::CreateShaderModule(std::vector<char> data, VkShaderModule& shaderModule)
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = data.size();
	info.pCode = reinterpret_cast<const uint32_t*>(data.data());
	auto result = vkCreateShaderModule(_device, &info, VK_NULL_HANDLE, &shaderModule);
	return result == VK_SUCCESS;
}

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

const ImWchar* GetGlyphRangesChineseFull()
{
	static const ImWchar ranges[] =
	{
		0x20, 0xFF, // Basic Latin + Latin Supplement
		0xff01, 0xff5e, // General Punctuation
		0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0xFF00, 0xFFEF, // Half-width characters
		0xFFFD, 0xFFFD, // Invalid
		0x4e00, 0x9FFF, // CJK Ideograms
		0x3400, 0x4DBF,
		0x2000, 0x20FF,
		0x3000,0x30FF,
		0x2100,0x21FF,
		0,
	};
	return &ranges[0];
}

ImGuiContext* VulkanManager::InitImgui_SDL(SDL_Window* handle, VkRenderPass renderPass, bool enableImguiDock, bool enableImguiMultiViewports, uint32_t subPassIndex)
{
#if ENABLE_IMGUI
	//Imgui
	ConsoleDebug::print_endl("hBBr:Start init imgui.");

	IMGUI_CHECKVERSION();
	auto newContent = ImGui::CreateContext();
	ImGui::SetCurrentContext(newContent);

	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	if (enableImguiDock)
	{
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
	}
	if (enableImguiMultiViewports)
	{
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	}
	#ifdef __ANDROID__
		ImFontConfig font_cfg;
		font_cfg.SizePixels = 22.0f;
		io.Fonts->AddFontDefault(&font_cfg);
		ImGui::GetStyle().ScaleAllSizes(3.0f);
	#endif
	io.IniFilename = "Config/GuiConfig.ini";

	// 设置字体大小
	io.Fonts->Clear();
	auto fontPath = FileSystem::Append(FileSystem::GetProgramPath(), GetRendererConfig("Default", "ImguiFontPath"));

	ImFontGlyphRangesBuilder fontBuilder = {};

	ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, nullptr,
		GetGlyphRangesChineseFull());

	if (!ImGui_ImplSDL3_InitForVulkan(handle))
		MessageOut("Error,ImGui_ImplSDL3_InitForVulkan return false!", false, true, "255,0,0");

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = _instance;
	init_info.PhysicalDevice = _gpuDevice;
	init_info.Device = _device;
	init_info.QueueFamily = _graphicsQueueFamilyIndex;
	init_info.Queue = _graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = _descriptorPool;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.MinImageCount = 2;
	init_info.ImageCount = _swapchainBufferCount;
	init_info.MinImageCount = _swapchainBufferCount;
	init_info.Subpass = subPassIndex;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;
	init_info.RenderPass = renderPass;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.CheckVkResultFn = nullptr;
	//init_info.UseDynamicRendering = true;

	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();

	return newContent;
#endif
}

void VulkanManager::ShutdownImgui()
{
#if ENABLE_IMGUI
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
#endif
}

void VulkanManager::ImguiNewFrame()
{
#if ENABLE_IMGUI
	ImGui_ImplVulkan_SetMinImageCount(_swapchainBufferCount);
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
#endif
}

void VulkanManager::ImguiEndDraw(VkCommandBuffer cmdBuf)
{
#if ENABLE_IMGUI
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
#endif
}

bool VulkanManager::ImguiEndFrame(VkSemaphore NeedWait)
{
#if ENABLE_IMGUI
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	bool bHasViewports = true;
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault(nullptr, NeedWait);
		if (platform_io.Viewports.Size <= 1)
		{
			bHasViewports = false;
		}
	}
	else
	{
		bHasViewports = false;
	}

	//没有多视口的模式下，Imgui是不会创建新的SubmitQueue和Swapchain
	if (NeedWait != nullptr && bHasViewports == false)
	{


	}
	return bHasViewports;
#endif
}

void VulkanManager::SubmitQueueImmediate(std::vector<VkCommandBuffer> cmdBufs, VkPipelineStageFlags waitStageMask, VkQueue queue)
{
	VkFence fence;
	CreateFence(fence, 0);

	if (cmdBufs.size() <= 0)
	{
		MessageOut(GetInternationalizationText("Renderer","A000013"), false, true);
	}
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pWaitDstStageMask = &waitStageMask;
	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = nullptr;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores = nullptr;
	info.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
	info.pCommandBuffers = cmdBufs.data();
	VkResult result;
	if (queue == VK_NULL_HANDLE)
		result = vkQueueSubmit(_graphicsQueue, 1, &info, fence);
	else
		result = vkQueueSubmit(queue, 1, &info, fence);
	if(result != VK_SUCCESS)
		MessageOut(("[Submit Queue Immediate]vkQueueSubmit error: " + GetVkResult(result)), false, false);

	vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX);

	DestroyFence(fence);
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

void VulkanManager::SubmitQueueForPasses(VkCommandBuffer& cmdBuf, VkSemaphore wait, VkSemaphore signal, VkFence executeFence, VkPipelineStageFlags waitStageMask, VkQueue queue)
{
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pWaitDstStageMask = &waitStageMask;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &wait;
	info.signalSemaphoreCount = 1;
	info.pSignalSemaphores = &signal;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &cmdBuf;
	VkResult result;
	if (queue == VK_NULL_HANDLE)
		result = vkQueueSubmit(_graphicsQueue, (uint32_t)1, &info, executeFence);
	else
		result = vkQueueSubmit(queue, (uint32_t)1, &info, executeFence);
	if (result != VK_SUCCESS)
		MessageOut((HString("[Submit Queue]vkQueueSubmit error : ") + GetVkResult(result)), false, true);
}

void VulkanManager::SubmitQueue(VkCommandBuffer& cmdBuf, std::vector<VkSemaphore> waits, std::vector<VkSemaphore> signals, VkFence executeFence, std::vector<VkPipelineStageFlags> waitStageMask, VkQueue queue)
{
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pWaitDstStageMask = waitStageMask.data();
	info.waitSemaphoreCount = (uint32_t)waits.size();
	info.pWaitSemaphores = waits.data();
	info.signalSemaphoreCount = (uint32_t)signals.size();;
	info.pSignalSemaphores = signals.data();
	info.commandBufferCount = 1;
	info.pCommandBuffers = &cmdBuf;
	VkResult result;
	if (queue == VK_NULL_HANDLE)
		result = vkQueueSubmit(_graphicsQueue, (uint32_t)1, &info, executeFence);
	else
		result = vkQueueSubmit(queue, (uint32_t)1, &info, executeFence);
	if (result != VK_SUCCESS)
		MessageOut((HString("[Submit Queue]vkQueueSubmit error : ") + GetVkResult(result)), false, true);
}

void VulkanManager::UpdateBufferDescriptorSet(VkBuffer buffer, VkDescriptorSet descriptorSet, VkDescriptorType type, uint32_t dstBinding, VkDeviceSize offset, VkDeviceSize Range)
{
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = Range;
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = dstBinding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = VK_NULL_HANDLE;
	descriptorWrite.pTexelBufferView = VK_NULL_HANDLE;
	vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, VK_NULL_HANDLE);
}

void VulkanManager::UpdateTextureDescriptorSet(VkDescriptorSet descriptorSet, std::vector<std::shared_ptr<Texture2D>> texs, std::vector<VkSampler> samplers, int beginBindingIndex)
{
	const uint32_t count = (uint32_t)texs.size();
	std::vector<VkWriteDescriptorSet> descriptorWrite(count);
	std::vector<VkDescriptorImageInfo> imageInfo(count);
	for (uint32_t o = 0; o < count; o++)
	{
		imageInfo[o] = {};
		imageInfo[o].sampler = samplers[o];
		imageInfo[o].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[o].imageView = texs[o]->GetTextureView();
		descriptorWrite[o] = {};
		descriptorWrite[o].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[o].dstSet = descriptorSet;
		descriptorWrite[o].dstBinding = beginBindingIndex + o;
		descriptorWrite[o].dstArrayElement = 0;
		descriptorWrite[o].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite[o].descriptorCount = 1;
		descriptorWrite[o].pBufferInfo = VK_NULL_HANDLE;
		descriptorWrite[o].pImageInfo = &imageInfo[o];
		descriptorWrite[o].pTexelBufferView = VK_NULL_HANDLE;
	}
	vkUpdateDescriptorSets(_device, (uint32_t)descriptorWrite.size(), descriptorWrite.data(), 0, VK_NULL_HANDLE);
}

void VulkanManager::UpdateTextureDescriptorSet(VkDescriptorSet descriptorSet, std::vector<TextureUpdateInfo> updateInfo, int beginBindingIndex)
{
	const uint32_t count = (uint32_t)updateInfo.size();
	std::vector<VkWriteDescriptorSet> descriptorWrite(count);
	std::vector<VkDescriptorImageInfo> imageInfo(count);
	for (uint32_t o = 0; o < count; o++)
	{
		imageInfo[o] = {};
		imageInfo[o].sampler = updateInfo[o].sampler;
		imageInfo[o].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[o].imageView = updateInfo[o].texture->GetTextureView();
		descriptorWrite[o] = {};
		descriptorWrite[o].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[o].dstSet = descriptorSet;
		descriptorWrite[o].dstBinding = beginBindingIndex + o;
		descriptorWrite[o].dstArrayElement = 0;
		descriptorWrite[o].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite[o].descriptorCount = 1;
		descriptorWrite[o].pBufferInfo = VK_NULL_HANDLE;
		descriptorWrite[o].pImageInfo = &imageInfo[o];
		descriptorWrite[o].pTexelBufferView = VK_NULL_HANDLE;
	}
	vkUpdateDescriptorSets(_device, (uint32_t)descriptorWrite.size(), descriptorWrite.data(), 0, VK_NULL_HANDLE);
}

void VulkanManager::UpdateStoreTextureDescriptorSet(VkDescriptorSet descriptorSet, std::vector<class Texture2D*> textures, int beginBindingIndex)
{
	const uint32_t count = (uint32_t)textures.size();
	std::vector<VkWriteDescriptorSet> descriptorWrite(count);
	std::vector<VkDescriptorImageInfo> imageInfo(count);
	for (uint32_t o = 0; o < count; o++)
	{
		imageInfo[o] = {};
		imageInfo[o].sampler = nullptr;
		imageInfo[o].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo[o].imageView = textures[o]->GetTextureView();
		descriptorWrite[o] = {};
		descriptorWrite[o].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[o].dstSet = descriptorSet;
		descriptorWrite[o].dstBinding = o + beginBindingIndex;
		descriptorWrite[o].dstArrayElement = 0;
		descriptorWrite[o].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorWrite[o].descriptorCount = 1;
		descriptorWrite[o].pBufferInfo = VK_NULL_HANDLE;
		descriptorWrite[o].pImageInfo = &imageInfo[o]; // Optional
		descriptorWrite[o].pTexelBufferView = VK_NULL_HANDLE; // Optional
	}
	vkUpdateDescriptorSets(VulkanManager::GetManager()->GetDevice(), count, descriptorWrite.data(), 0, VK_NULL_HANDLE);
}

VkDeviceSize VulkanManager::GetMinUboAlignmentSize(VkDeviceSize realSize)
{
	VkDeviceSize outSize = realSize;
	VkDeviceSize minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
	if (minUboAlignment > 0) {
		outSize = (outSize + (VkDeviceSize)minUboAlignment - 1) & ~((VkDeviceSize)minUboAlignment - 1);
	}
	return outSize;
}

VkDeviceSize VulkanManager::GetMinTboAlignmentSize(VkDeviceSize realSize)
{
	VkDeviceSize outSize = realSize;
	VkDeviceSize minUboAlignment = _gpuProperties.limits.minTexelBufferOffsetAlignment;
	if (minUboAlignment > 0) {
		outSize = (outSize + (VkDeviceSize)minUboAlignment - 1) & ~((VkDeviceSize)minUboAlignment - 1);
	}
	return outSize;
}

VkDeviceSize VulkanManager::GetMinSboAlignmentSize(VkDeviceSize realSize)
{
	VkDeviceSize outSize = realSize;
	VkDeviceSize minUboAlignment = _gpuProperties.limits.minStorageBufferOffsetAlignment;
	if (minUboAlignment > 0) {
		outSize = (outSize + (VkDeviceSize)minUboAlignment - 1) & ~((VkDeviceSize)minUboAlignment - 1);
	}
	return outSize;
}

float VulkanManager::GetTimestampPeriod()
{
	return _gpuProperties.limits.timestampPeriod;
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

void VulkanManager::CmdColorBitImage(VkCommandBuffer cmdBuf, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D targetSize, VkFilter filter)
{
	VkImageBlit region;
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.mipLevel = 0;
	region.srcSubresource.baseArrayLayer = 0;
	region.srcSubresource.layerCount = 1;
	// 指定要传输的数据所在的三维图像区域
	region.srcOffsets[0].x = 0;
	region.srcOffsets[0].y = 0;
	region.srcOffsets[0].z = 0;
	// [1] z轴都是1
	region.srcOffsets[1].x = srcSize.width;
	region.srcOffsets[1].y = srcSize.height;
	region.srcOffsets[1].z = 1;
	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.mipLevel = 0;
	region.dstSubresource.baseArrayLayer = 0;
	region.dstSubresource.layerCount = 1;
	region.dstOffsets[0].x = 0;
	region.dstOffsets[0].y = 0;
	region.dstOffsets[0].z = 0;
	region.dstOffsets[1].x = std::min(targetSize.width, srcSize.width);
	region.dstOffsets[1].y = std::min(targetSize.height, srcSize.height);
	region.dstOffsets[1].z = 1;
	//vkCmdBlitImage(_renderer->GetCommandBuffer(), src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &region, VK_FILTER_LINEAR);
	vkCmdBlitImage(cmdBuf, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, filter);
}

void VulkanManager::CmdBufferCopyToBuffer(VkCommandBuffer cmdBuf, VkBuffer src, VkBuffer dst, VkDeviceSize copySize, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
	bool bNoCmdBuf = (cmdBuf == VK_NULL_HANDLE || cmdBuf == nullptr);
	if (bNoCmdBuf)
	{
		AllocateCommandBuffer(_commandPool, cmdBuf);
		BeginCommandBuffer(cmdBuf, 0);
	}

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = copySize;
	vkCmdCopyBuffer(cmdBuf, src, dst, 1, &copyRegion);

	if (bNoCmdBuf)
	{
		EndCommandBuffer(cmdBuf);
		SubmitQueueImmediate({ cmdBuf });
		vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
		FreeCommandBuffer(_commandPool, cmdBuf);
	}
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