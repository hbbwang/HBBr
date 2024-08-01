#pragma once
//Vulkan底层核心管理类

//使用IMGUI
#define ENABLE_IMGUI 1

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

#include <vma/vk_mem_alloc.h>

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
	uint8_t HasEXTFullscreenExclusive = 0;
	uint8_t HasKHRShaderFloatControls = 0;
};

struct BufferUpdateInfo
{
	VkDeviceSize offset;
	VkDeviceSize range;
	BufferUpdateInfo(
		VkDeviceSize _offset,
		VkDeviceSize _range)
	{
		offset = _offset;
		range = _range;
	}
};

struct TextureUpdateInfo
{
	std::shared_ptr<class Texture2D>texture;
	VkSampler sampler;
	TextureUpdateInfo(
		std::shared_ptr<class Texture2D>_texture,
		VkSampler _sampler)
	{
		texture = _texture;
		sampler = _sampler;
	}
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

	HBBR_API void DeviceWaitIdle();

	HBBR_API void QueueWaitIdle(VkQueue queue);

	HBBR_API HBBR_INLINE VmaAllocator GetVMA()const {
		return _vma_allocator;
	}

	/* 初始化Vulkan */

	void InitInstance(bool bEnableDebug);

	void InitDevice();

	void InitDebug();

	void InitVMA();

	/* 创建Surface */
	void ReCreateSurface_SDL(SDL_Window* handle, VkSurfaceKHR& newSurface);

	/* 释放Surface */
	void DestroySurface(VkSurfaceKHR& surface);

	/* 获取Surface的大小 */
	void GetSurfaceSize(VkSurfaceKHR surface, VkExtent2D& surfaceSize);

	/* 检查Surface支持 */
	void CheckSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR& surfaceFormat);

	bool GetSurfaceCapabilities(VkSurfaceKHR& surface, VkSurfaceCapabilitiesKHR* surfaceCapabilities);

	/* 创建Swapchain */
	VkExtent2D CreateSwapchain(
		SDL_Window* window,
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

	/* 释放Swapchain */
	void DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<VkImageView>& swapchainImageViews);

	/* 创建Vulkan image ,但是不带 mipmaps */
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, VkImage& image ,uint32_t miplevel = 1, uint32_t layerCount = 1);

	/* 根据VkImageView ,创建Vulkan image view*/
	void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,  VkImageView& imageView, uint32_t miplevel = 1, uint32_t layerCount = 1);

	/* 创建Vulkan image view memory*/
	VkDeviceSize CreateImageMemory(VkImage image, VkDeviceMemory& imageViewMemory, VkMemoryPropertyFlags memoryPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	bool CheckImageProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usages ,VkImageCreateFlags flags, VkImageFormatProperties* out = nullptr);

	void DestroyImage(VkImage& inImage);

	void DestroyImageView(VkImageView& imageView);

	void CreateSampler(VkSampler& sampler, VkFilter filter = VK_FILTER_LINEAR, VkSamplerAddressMode address = VK_SAMPLER_ADDRESS_MODE_REPEAT, float minMipLeve = 0, float maxMipLevel = 16);

	/* 创建Frame buffer */
	void CreateFrameBuffers(VkExtent2D FrameBufferSize, VkRenderPass renderPass, std::vector<VkImageView> attachments, std::vector<VkFramebuffer>& frameBuffers);

	void DestroyFrameBuffers(std::vector<VkFramebuffer>& frameBuffers);

	void CreateCommandPool();

	void ResetCommandPool();

	void DestroyCommandPool();

	void CreateCommandPool(VkCommandPool& commandPool , VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	void ResetCommandPool(VkCommandPool& commandPool);

	void DestroyCommandPool(VkCommandPool commandPool);

	void AllocateCommandBuffer(VkCommandPool commandPool , VkCommandBuffer& cmdBuf);

	void FreeCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer>& cmdBufs);

	void FreeCommandBuffer(VkCommandPool commandPool, VkCommandBuffer& cmdBuf);

	void ResetCommandBuffer(VkCommandBuffer& cmdBuf);

	void BeginCommandBuffer(VkCommandBuffer& cmdBuf , VkCommandBufferUsageFlags flag = 0);

	void EndCommandBuffer(VkCommandBuffer& cmdBuf);

	void BeginRenderPass(VkCommandBuffer& cmdBuf, VkFramebuffer framebuffer, VkRenderPass renderPass, VkExtent2D areaSize, std::vector<VkAttachmentDescription>_attachmentDescs, std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });

	void EndRenderPass(VkCommandBuffer& cmdBuf);

	/* return the swapchain is normal (not out of data). */
	bool GetNextSwapchainIndex(VkSwapchainKHR& swapchain, VkSemaphore& semaphore, VkFence* fence , uint32_t* swapchainIndex);

	bool Present(VkSwapchainKHR& swapchain, VkSemaphore& semaphore, uint32_t& swapchainImageIndex);

	void ReCreatePipelineLayout(std::vector <VkDescriptorSetLayout> descriptorSetLayout, VkPipelineLayout& pipelineLayout);

	void CreatePipelineLayout(std::vector <VkDescriptorSetLayout> descriptorSetLayout , VkPipelineLayout& pipelineLayout);

	void DestroyPipelineLayout(VkPipelineLayout& pipelineLayout);

	void CreateDescripotrPool(VkDescriptorPool& pool);

	void DestroyDescriptorPool(VkDescriptorPool& pool);

	void CreateDescripotrSetLayout(VkDescriptorType type, uint32_t bindingCount ,  VkDescriptorSetLayout& descriptorSetLayout , VkShaderStageFlags shaderStageFlags);

	void CreateDescripotrSetLayout(std::vector<VkDescriptorType> types, VkDescriptorSetLayout& descriptorSetLayout, VkShaderStageFlags shaderStageFlags);

	void CreateDescripotrSetLayout(std::vector<VkDescriptorType> types, std::vector<VkShaderStageFlags> shaderStageFlags, VkDescriptorSetLayout& descriptorSetLayout);

	void DestroyDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout);

	/* width and height must be same as attachments(ImageViews) size. */
	void CreateFrameBuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, std::vector<VkImageView>attachments, VkFramebuffer& framebuffer);

	void DestroyFrameBuffer(VkFramebuffer& framebuffer);

	/* Allocate a new descriptorSet ,attention,we should be free the old or unuseful descriptorSet for save memory. */
	void AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet);

	void AllocateDescriptorSets(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VkDescriptorSet>& descriptorSet);

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

	void CreateFence(VkFence& fence, VkFenceCreateFlags createFlags = VK_FENCE_CREATE_SIGNALED_BIT);

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

	void CreateComputePipeline(VkComputePipelineCreateInfo& info, VkPipeline& pipeline);

	void DestroyPipeline(VkPipeline& pipeline);

	void CreateRenderPass(std::vector<VkAttachmentDescription>attachmentDescs, std::vector<VkSubpassDependency>subpassDependencys, std::vector<VkSubpassDescription>subpassDescs, VkRenderPass& renderPass);

	void DestroyRenderPass(VkRenderPass& renderPass);

	void CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize bufferSize, VkBuffer& buffer);

	void AllocateBufferMemory(VkBuffer buffer , VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void CreateBufferAndAllocateMemory(size_t bufferSize, uint32_t bufferUsage, uint32_t bufferMemoryProperty, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	//VMA_MEMORY_USAGE_UNKNOWN					
	// 没有预设的内存使用，可以使用VmaAllocationCreateInfo当中的其他字段来对内存进行规范（比如preferrableFlags或者requiredFlags)
	// 
	//VMA_MEMORY_USAGE_GPU_ONLY					
	// 内存只能被GPU使用，在GPU上访问速度是最快的；也意味着从显存（VRAM）上面进行分配，不需要在host端做mapping操作。用途：GPU对其进行读写的设备，比如作为attachments的图片。只会从CPU端传输一次或者特别低频的数据，而且这个数据会被GPU高频读取，比如纹理贴图，VBO，UniformBuffer（注意前面说了CPU低频）以及大部分的只在GPU使用的资源。在某些情况下（比如AMD的256M的VRAM）可以使用HOST_VISIBLE与此并存，所以也可以进行Map后来直接操作数据。
	// 
	//VMA_MEMORY_USAGE_CPU_ONLY					
	// 此类内存可以直接通过Map到CPU端进行操作，这个usage绝对保证是HOST_VISIBLE并且是HOST_COHERENT（即时同步），CPU端是uncached状态，就是说不需要flush到GPU。写入操作有可能是Write - Combined类型（也就是说写入一堆数据一起送入GPU）。GPU端也是可以自由访问这种内存，但是会比较慢。用途：用于StagingBuffer（中间传输跳板）
	// 
	//VMA_MEMORY_USAGE_CPU_TO_GPU				
	// 此类内存可以从CPU进行Map操作后访问（一定是HOST_VISIBLE），并且尽可能快速地从GPU访问。CPU段Uncached，写入操作可能是Write - Combined。用途：CPU端频繁进行数据写入，GPU端对数据进行读取。比如说纹理，VBO，UniformBuffers，这些Buffer如果每一帧都更新，可以这么使用。
	// 
	//VMA_MEMORY_USAGE_GPU_TO_CPU				
	// 此类内存可以通过Map操作在CPU访问（一定是HOST_VISIBLE）并且是拥有Cached状态。用途：被GPU写入并且被CPU读取的buffer / image，比如录屏，HDR的全屏操作等。任何在CPU端随机访问的资源。
	// 
	//VMA_MEMORY_USAGE_CPU_COPY					
	// CPU的内存，尽可能不是GPU可访问的，但是一定是HOST_VISIBLE。用途：属于GPU当中，由于换页等操作，临时挪动到CPU的资源，当需要的时候会被换回到GPU显存
	// 
	//VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED		
	// 懒惰分配的内存，也就是及说GPU端Memory在分配的时候使用了VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT标识符，也就是按需分配，如果内存不被访问，即便是调用了接口也不会申请内存分配。注意：如果是没有这种标识符对应类型的内存，就会分配失败哦。用途：临时的图片等，比如MSAA抗锯齿的时候会用到，因为可能关掉抗锯齿。此类图片内存一定有关键字：VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
	//
	void VMACraeteBufferAndAllocateMemory(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationInfo* vmaInfo = nullptr, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY, bool bAlwayMapping = false, bool bFocusCreateDedicatedMemory = false);

	void VMADestroyBufferAndFreeMemory(VkBuffer& buffer, VmaAllocation& allocation, HString debugName = "VMABuffer", VkDeviceSize debugSize = 0
	);

	void DestroyBufferAndMemory(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void FreeBufferMemory(VkDeviceMemory& bufferMemory);

	void DestroyBuffer(VkBuffer& buffer);

	bool CreateShaderModule(std::vector<char> data , VkShaderModule& shaderModule);

	struct ImGuiContext* InitImgui_SDL(SDL_Window* handle, VkRenderPass renderPass, uint32_t subPassIndex = 0 );

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

	void SubmitQueueForPasses(VkCommandBuffer& cmdBuf, VkSemaphore& presentSemaphore, VkSemaphore& submitFinishSemaphore, VkFence& executeFence , VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VkQueue queue = VK_NULL_HANDLE);

	void UpdateBufferDescriptorSet(VkBuffer buffer, VkDescriptorSet descriptorSet, VkDescriptorType type,  uint32_t dstBinding, VkDeviceSize offset, VkDeviceSize Range);

	void UpdateTextureDescriptorSet(VkDescriptorSet descriptorSet, std::vector<std::shared_ptr<Texture2D>> texs, std::vector<VkSampler>samplers, int beginBindingIndex = 0);

	void UpdateTextureDescriptorSet(VkDescriptorSet descriptorSet, std::vector<TextureUpdateInfo> updateInfo, int beginBindingIndex = 0);

	void UpdateStoreTextureDescriptorSet(VkDescriptorSet descriptorSet, std::vector<class Texture2D*> textures, int beginBindingIndex = 0);

	VkDeviceSize GetMinUboAlignmentSize(VkDeviceSize realSize);

	VkDeviceSize GetMinTboAlignmentSize(VkDeviceSize realSize);

	VkDeviceSize GetMinSboAlignmentSize(VkDeviceSize realSize);

	VkDeviceSize GetTimestampPeriod();

	inline uint32_t GetMaxUniformBufferSize()const {
		return _gpuProperties.limits.maxUniformBufferRange;
	}

	inline VkPhysicalDeviceProperties GetPhysicalDeviceProperties()const {
		return _gpuProperties;
	}

	/* CMD */
	void CmdSetViewport(VkCommandBuffer cmdbuf, std::vector<VkExtent2D> viewports);

	void CmdNextSubpass(VkCommandBuffer cmdbuf, VkSubpassContents subpassContents = VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

	void CmdCmdBindPipeline(VkCommandBuffer cmdbuf ,VkPipeline pipelineObject, VkPipelineBindPoint bindPoint);

	void CmdColorBitImage(VkCommandBuffer cmdBuf, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D targetSize);

	void CmdBufferCopyToBuffer(VkCommandBuffer cmdBuf, VkBuffer src, VkBuffer dst, VkDeviceSize copySize, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);

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

	//计算出来的是纳秒,转毫秒需要 (double)/1000000.0
	HBBR_INLINE double CalculateTimestampQuery(uint32_t firstIndex, uint32_t CalculateCount)const {
		uint64_t timestamps[2];
		vkGetQueryPoolResults(_device, _queryTimeStamp, firstIndex, CalculateCount, sizeof(timestamps), timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
		uint64_t elapsedTime = timestamps[1] - timestamps[0];
		return (double)elapsedTime * (double)_gpuProperties.limits.timestampPeriod;
	}

	void CreateQueryPool(uint32_t queryCount, VkQueryPool& poolInOut, VkQueryType type = VK_QUERY_TYPE_TIMESTAMP);

	void DestroyQueryPool(VkQueryPool& poolInOut);

	
	VkQueryPool GetQueryTimestamp()const {
		return _queryTimeStamp;
	}

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

	VmaAllocator _vma_allocator;

	VkDebugReportCallbackCreateInfoEXT	debugCallbackCreateInfo{};

	VkDebugReportCallbackEXT			_debugReport;

	VkPhysicalDeviceProperties			_gpuProperties{};

	VkPhysicalDeviceMemoryProperties	_gpuMemoryProperties{};

	VkPhysicalDeviceFeatures			_gpuFeatures{};

	VkPhysicalDeviceVulkan12Features	_gpuVk12Features{};

	VkQueue	_graphicsQueue;

	VkCommandPool _commandPool;

	VkDescriptorPool _descriptorPool;

	int _graphicsQueueFamilyIndex;

	uint32_t _swapchainBufferCount;

#ifdef _WIN32
	bool _enable_VK_KHR_display;
#endif

	VkQueryPool _queryTimeStamp = VK_NULL_HANDLE;

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