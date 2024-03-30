#pragma once
//Vulkan底层核心管理类

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
	#define VK_USE_PLATFORM_WIN32_KHR	1	//support win32
	#include <stdlib.h>
#elif defined(__ANDROID__)
	#define VK_USE_PLATFORM_ANDROID_KHR 1
#elif defined(__linux__)
	#define VK_USE_PLATFORM_XCB_KHR 1
#elif defined(__ios__)
	#define VK_USE_PLATFORM_IOS_MVK 1
#elif defined(__macos__)
	#define VK_USE_PLATFORM_MACOS_MVK 1
#endif

#if defined(__ANDROID__)
#include "vulkan_wrapper/vulkan_wrapper.h"
#endif

#include "GLFWInclude.h"
#include <vulkan/vulkan.h>
#include "../Common/Common.h"
#include <memory>
#include <array>
#include "HString.h"
#include "Thread.h"

#define COMMAND_MAKER(cmdBuf ,UniqueID ,name, color) \
	struct Renderer_Command_Maker_##UniqueID\
	{\
		Renderer_Command_Maker_##UniqueID(VkCommandBuffer cmdBufIn ,const char* passName, glm::vec4 makerColor)\
		{\
			_cmdBuf = cmdBufIn;\
			VulkanManager::BeginRegion(_cmdBuf, passName, makerColor);\
		}\
		~Renderer_Command_Maker_##UniqueID()\
		{\
			VulkanManager::EndRegion(_cmdBuf);\
		}\
		VkCommandBuffer _cmdBuf;\
	};\
	Renderer_Command_Maker_##UniqueID _renderer_Command_Maker_##UniqueID(cmdBuf,name,color);

enum class EPlatform :uint8_t
{
	Windows		= 0,
	Linux		= 1,
	Android		= 2,
};

struct OptionalVulkanDeviceExtensions
{
	uint8_t HasKHRDebugMarker = 0;
	uint8_t HasKHRSwapchain = 0;
	uint8_t HasExtendedDynamicState = 0;
	uint8_t HasKHRMaintenance1 = 0;
	uint8_t HasKHRMaintenance2 = 0;
	uint8_t HasMemoryBudget = 0;
	uint8_t HasMemoryPriority = 0;
	uint8_t HasKHRDedicatedAllocation = 0;
	uint8_t HasKHRDedicatedHostOperations = 0;
	uint8_t HasKHRSpirv_1_4 = 0;
	uint8_t HasKHRImageFormatList = 0;
	uint8_t HasEXTValidationCache = 0;
	uint8_t HasAMDBufferMarker = 0;
	uint8_t HasKHRExternalFence = 0;
	uint8_t HasKHRExternalSemaphore = 0;
	uint8_t HasKHRHuaWeiSubpassShading = 0;
	uint8_t HasKHRHuaWeiSmartCache = 0;
	uint8_t HasQcomRenderPassTransform = 0;
	uint8_t HasKHRCreateRenderPass2 = 0;
	uint8_t HasKHRSeparateDepthStencilLayouts = 0;
	uint8_t HasEXTFullscreenExclusive = 1;
	uint8_t HasKHRShaderFloatControls = 0;
};

class VulkanManager
{

public:
	VulkanManager(bool bDebug);
	~VulkanManager();

	HBBR_API HBBR_INLINE static void InitManager(bool bDebug) {
		if (_vulkanManager == nullptr)
		{
			_vulkanManager.reset(new VulkanManager(bDebug));
		}
	}

	HBBR_API HBBR_INLINE static VulkanManager* GetManager() {
		return _vulkanManager.get();
	}

	HBBR_API static void ReleaseManager() {
		if (_vulkanManager)
		{
			_vulkanManager.reset();
		}
	}

	HBBR_INLINE VkQueue GetGraphicsQueue() {
		return _graphicsQueue;
	}

	/* 初始化Vulkan */

	void InitInstance(bool bEnableDebug);

	void InitDevice();

	void InitDebug();

	/* 创建Surface */
	void ReCreateSurface_SDL(SDL_Window* handle, VkSurfaceKHR& newSurface);

	/* 释放Surface */
	void DestroySurface(VkSurfaceKHR& surface);

	/* 获取Surface的大小 */
	VkExtent2D GetSurfaceSize(VkExtent2D windowSizeIn, VkSurfaceKHR surface);

	/* 检查Surface支持 */
	void CheckSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR& surfaceFormat);

	/* 创建Swapchain */
	VkExtent2D CreateSwapchain(
		VkExtent2D surfaceSize,
		VkSurfaceKHR surface,
		VkSurfaceFormatKHR surfaceFormat,
		VkSwapchainKHR& newSwapchain,
		std::vector<VkImage>& swapchainImages,
		std::vector<VkImageView>& swapchainImageViews,
		VkSurfaceCapabilitiesKHR& surfaceCapabilities,
		std::vector<VkCommandBuffer>* cmdBuf,
		std::vector<VkSemaphore>* acquireImageSemaphore ,
		std::vector<VkSemaphore>* queueSubmitSemaphore,
		std::vector<VkFence>* fences = nullptr,
		bool bIsFullScreen = false,
		bool bVSync = true
	);

	/* 创建Swapchain From Texture2D Class */
	VkExtent2D CreateSwapchainFromTextures(VkExtent2D surfaceSize, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSwapchainKHR& newSwapchain, std::vector<std::shared_ptr<class Texture2D>>& textures , std::vector<VkImageView>& swapchainImageViews);

	/* 释放Swapchain */
	void DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<VkImageView>& swapchainImageViews);

	/* 释放Swapchain From Texture2D Class */
	void DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<std::shared_ptr<class Texture2D>>& textures);

	/* 创建Vulkan image ,但是不带 mipmaps */
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, VkImage& image ,uint32_t miplevel = 1, uint32_t layerCount = 1);

	/* 根据VkImageView ,创建Vulkan image view*/
	void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,  VkImageView& imageView, uint32_t miplevel = 1, uint32_t layerCount = 1);

	/* 创建Vulkan image view memory*/
	VkDeviceSize CreateImageMemory(VkImage image, VkDeviceMemory& imageViewMemory, VkMemoryPropertyFlags memoryPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	void DestroyImage(VkImage& inImage);

	void DestroyImageView(VkImageView& imageView);

	void CreateSampler(VkSampler& sampler, VkFilter filter = VK_FILTER_LINEAR, VkSamplerAddressMode address = VK_SAMPLER_ADDRESS_MODE_REPEAT, float minMipLeve = 0, float maxMipLevel = 16);

	/* 创建Frame buffer */
	void CreateFrameBuffers(VkExtent2D FrameBufferSize, VkRenderPass renderPass, std::vector<VkImageView> attachments, std::vector<VkFramebuffer>& frameBuffers);

	void DestroyFrameBuffers(std::vector<VkFramebuffer>& frameBuffers);

	void CreateCommandPool();

	void ResetCommandPool();

	void DestroyCommandPool();

	void CreateCommandPool(VkCommandPool& commandPool);

	void ResetCommandPool(VkCommandPool& commandPool);

	void DestroyCommandPool(VkCommandPool commandPool);

	void AllocateCommandBuffer(VkCommandPool commandPool , VkCommandBuffer& cmdBuf);

	void FreeCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer> cmdBufs);

	void ResetCommandBuffer(VkCommandBuffer cmdBuf);

	void BeginCommandBuffer(VkCommandBuffer cmdBuf , VkCommandBufferUsageFlags flag = 0);

	void EndCommandBuffer(VkCommandBuffer cmdBuf);

	void BeginRenderPass(VkCommandBuffer cmdBuf, VkFramebuffer framebuffer, VkRenderPass renderPass, VkExtent2D areaSize, std::vector<VkAttachmentDescription>_attachmentDescs, std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });

	void EndRenderPass(VkCommandBuffer cmdBuf);

	/* return the swapchain is normal (not out of data). */
	bool GetNextSwapchainIndex(VkSwapchainKHR swapchain, VkSemaphore semaphore,VkFence fence , uint32_t* swapchainIndex);

	bool Present(VkSwapchainKHR swapchain, VkSemaphore& semaphore, uint32_t& swapchainImageIndex);

	void CreatePipelineLayout(std::vector <VkDescriptorSetLayout> descriptorSetLayout , VkPipelineLayout& pipelineLayout);

	void DestroyPipelineLayout(VkPipelineLayout& pipelineLayout);

	void CreateDescripotrPool(VkDescriptorPool& pool);

	void DestroyDescriptorPool(VkDescriptorPool& pool);

	void CreateDescripotrSetLayout(VkDescriptorType type, uint32_t bindingCount ,  VkDescriptorSetLayout& descriptorSetLayout , VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	void CreateDescripotrSetLayout(std::vector<VkDescriptorType> types, VkDescriptorSetLayout& descriptorSetLayout, VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	void DestroyDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout);

	/* width and height must be same as attachments(ImageViews) size. */
	void CreateFrameBuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, std::vector<VkImageView>attachments, VkFramebuffer& framebuffer);

	void DestroyFrameBuffer(VkFramebuffer& framebuffer);

	/* Allocate a new descriptorSet ,attention,we should be free the old or unuseful descriptorSet for save memory. */
	void AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet);

	void AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, uint32_t newDescriptorSetCount, std::vector<VkDescriptorSet>& descriptorSet);

	/* Pool must created setting VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT */
	void FreeDescriptorSet(VkDescriptorPool pool, std::vector<VkDescriptorSet>& descriptorSet);

	void FreeDescriptorSet(VkDescriptorPool pool, VkDescriptorSet& descriptorSet);

	/* Image 布局转换 */
	void Transition(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspects, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);

	void CreateVkSemaphore(VkSemaphore& semaphore);

	void DestroySemaphore(VkSemaphore& semaphore);

	void CreateRenderSemaphores(std::vector<VkSemaphore>& semaphore);

	void DestroyRenderSemaphores(std::vector<VkSemaphore>& semaphore);

	void CreateFence(VkFence& fence);

	void RecreateFences(std::vector<VkFence>& fences , uint32_t number);

	void DestroyFence(VkFence& fence);

	void ResetFence(VkFence& fence);

	bool IsFenceFinish(VkFence& fence);

	void CreateRenderFences(std::vector<VkFence>& fences);

	void DestroyRenderFences(std::vector<VkFence>& fences);

	void WaitForFences(std::vector<VkFence> fences, bool bReset = true, uint64_t timeOut = UINT64_MAX);

	//Android 不支持...？
	//void WaitSemaphores(std::vector<VkSemaphore> semaphores , uint64_t timeOut = UINT64_MAX);

	void CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo& info , VkPipeline& pipeline);

	void DestroyPipeline(VkPipeline& pipeline);

	void CreateRenderPass(std::vector<VkAttachmentDescription>attachmentDescs, std::vector<VkSubpassDependency>subpassDependencys, std::vector<VkSubpassDescription>subpassDescs, VkRenderPass& renderPass);

	void DestroyRenderPass(VkRenderPass& renderPass);

	void CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize bufferSize, VkBuffer& buffer);

	void AllocateBufferMemory(VkBuffer buffer , VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void CreateBufferAndAllocateMemory(size_t bufferSize, uint32_t bufferUsage, uint32_t bufferMemoryProperty, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void DestroyBufferAndMemory(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void FreeBufferMemory(VkDeviceMemory& bufferMemory);

	void DestroyBuffer(VkBuffer& buffer);

	bool CreateShaderModule(std::vector<char> data , VkShaderModule& shaderModule);

	void InitImgui_SDL(SDL_Window* handle , VkRenderPass renderPass , uint32_t subPassIndex = 0);

	void ResetImgui_SDL( VkRenderPass renderPass, uint32_t subPassIndex = 0 , glm::mat4 projMat = 
		glm::mat4(
			glm::vec4(1, 0, 0, 0),
			glm::vec4(0, 1, 0, 0),
			glm::vec4(0, 0, 1, 0),
			glm::vec4(0, 0, 0, 1)
		)
	);

	void ShutdownImgui();

	void ImguiNewFrame();

	void ImguiEndFrame(VkCommandBuffer cmdBuf);

	/* 立刻序列提交,为保证运行安全,会执行一次等待运行结束 */
	void SubmitQueueImmediate(std::vector<VkCommandBuffer> cmdBufs, VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkQueue queue = VK_NULL_HANDLE);

	void SubmitQueue(std::vector<VkCommandBuffer> cmdBufs, std::vector <VkSemaphore> lastSemaphore, std::vector <VkSemaphore> newSemaphore, VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkQueue queue = VK_NULL_HANDLE);

	VkViewport GetViewport(float w,float h);

	void SubmitQueueForPasses(VkCommandBuffer cmdBuf,std::vector<std::shared_ptr<class PassBase>> passes, VkSemaphore* presentSemaphore, VkSemaphore* submitFinishSemaphore, VkFence executeFence , VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VkQueue queue = VK_NULL_HANDLE);

	void UpdateBufferDescriptorSet(class DescriptorSet* descriptorSet, uint32_t dstBinding, VkDeviceSize offset , VkDeviceSize Range);

	void UpdateBufferDescriptorSet(class DescriptorSet* descriptorSet, uint32_t dstBinding, uint32_t sameBufferSize, std::vector<uint32_t> offsets);

	void UpdateBufferDescriptorSet(class DescriptorSet* descriptorSet, uint32_t dstBinding, std::vector<uint32_t>bufferSizes, std::vector<uint32_t> offsets);

	void UpdateBufferDescriptorSetAll(class DescriptorSet* descriptorSet, uint32_t dstBinding, VkDeviceSize offset, VkDeviceSize Range);

	void UpdateTextureDescriptorSet(VkDescriptorSet descriptorSet, std::vector<class Texture2D*> textures, std::vector<VkSampler> samplers);

	VkDeviceSize GetMinUboAlignmentSize(VkDeviceSize realSize);

	/* CMD */
	void CmdSetViewport(VkCommandBuffer cmdbuf, std::vector<VkExtent2D> viewports);

	void CmdNextSubpass(VkCommandBuffer cmdbuf, VkSubpassContents subpassContents = VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

	void CmdCmdBindPipeline(VkCommandBuffer cmdbuf ,VkPipeline pipelineObject, VkPipelineBindPoint bindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS);

	//RenderDoc debug
	static void SetObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char* name);
	static void SetObjectTag(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag);
	static void BeginRegion(VkCommandBuffer cmdbuf, const char* pMarkerName, glm::vec4 color);
	static void InsertRegion(VkCommandBuffer cmdbuf, std::string markerName, glm::vec4 color);
	static void EndRegion(VkCommandBuffer cmdbuf);

	/*-----------------*/

	/* 获取平台 */
	HBBR_INLINE EPlatform GetPlatform()const {
		return _currentPlatform;
	}

	HBBR_INLINE VkInstance GetInstance()const {
		return _instance;
	}

	HBBR_INLINE uint32_t GetSwapchainBufferCount()const {
		return _swapchainBufferCount;
	}

	HBBR_INLINE VkDevice GetDevice()const {
		return _device;
	}

	HBBR_INLINE VkPhysicalDevice GetPhysicalDevice()const {
		return _gpuDevice;
	}

	HBBR_INLINE VkCommandPool GetCommandPool()const {
		return _commandPool;
	}

	HBBR_INLINE VkDescriptorPool GetDescriptorPool()const {
		return _descriptorPool;
	}

	HBBR_INLINE VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures()const {
		return _gpuFeatures;
	}

	HBBR_INLINE VkDescriptorSetLayout GetImageDescriptorSetLayout() {
		if (_descriptorSetLayout_tex == VK_NULL_HANDLE)
		{
			CreateDescripotrSetLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, _descriptorSetLayout_tex);
		}
		return _descriptorSetLayout_tex;
	}

	//忽略vkGetSwapchainImagesKHR里获取到的Images数量，坚持使用我们设定的数量
	bool _bIsIgnoreVulkanSwapChainExtraImages = false;

	static bool _bDebugEnable;

private:

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

	VkPhysicalDeviceProperties			_gpuProperties{};

	VkPhysicalDeviceMemoryProperties	_gpuMemoryProperties{};

	VkPhysicalDeviceFeatures			_gpuFeatures{};

	VkPhysicalDeviceVulkan12Features	_gpuVk12Features{};

	VkDescriptorSetLayout				_descriptorSetLayout_tex = VK_NULL_HANDLE;

	VkQueue	_graphicsQueue;

	VkCommandPool _commandPool;

	VkDescriptorPool _descriptorPool;

	int _graphicsQueueFamilyIndex;

	uint32_t _swapchainBufferCount;

#ifdef _WIN32
	bool _enable_VK_KHR_display;
#endif

	OptionalVulkanDeviceExtensions _deviceExtensionOptionals;

	VkSurfaceTransformFlagBitsKHR QCOMRenderPassTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	//Debug marker
	static bool debugMarkerActive;
	static bool extensionPresent;
	static PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag;
	static PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName;
	static PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin;
	static PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd;
	static PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert;
	static std::unique_ptr<VulkanManager> _vulkanManager;
};