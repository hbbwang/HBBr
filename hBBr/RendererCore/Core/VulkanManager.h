#pragma once
//Vulkan底层核心管理类
#include "../Common/Common.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <array>
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
	VulkanManager(bool bDebug);
	~VulkanManager();

	HBBR_API __forceinline static void InitManager(bool bDebug) {
		if (_vulkanManager == NULL)
		{
			_vulkanManager.reset(new VulkanManager(bDebug));
		}
	}

	HBBR_API __forceinline static std::shared_ptr<VulkanManager> GetManager() {
		return _vulkanManager;
	}

	__forceinline static void ReleaseManager() {
		if (_vulkanManager)
		{
			_vulkanManager.reset();
		}
	}

	__forceinline VkQueue GetGraphicsQueue() {
		return _graphicsQueue;
	}

	/* 初始化Vulkan */

	void InitInstance(bool bEnableDebug);

	void InitDevice();

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

	/* 创建Swapchain From Texture Class */
	VkExtent2D CreateSwapchain(VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSwapchainKHR& newSwapchain, std::vector<std::shared_ptr<class Texture>>& textures);

	/* 释放Swapchain */
	void DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews);

	/* 释放Swapchain From Texture Class */
	void DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<std::shared_ptr<class Texture>>& textures);

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

	void DestroyFrameBuffers(std::vector<VkFramebuffer>& frameBuffers);

	void CreateCommandPool();

	void DestroyCommandPool();

	void CreateCommandPool(VkCommandPool& commandPool);

	void DestroyCommandPool(VkCommandPool commandPool);

	void AllocateCommandBuffer(VkCommandPool commandPool , VkCommandBuffer& cmdBuf);

	void FreeCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer> cmdBufs);

	void BeginCommandBuffer(VkCommandBuffer cmdBuf , VkCommandBufferUsageFlags flag = 0);

	void EndCommandBuffer(VkCommandBuffer cmdBuf);

	void BeginRenderPass(VkCommandBuffer cmdBuf, VkFramebuffer framebuffer, VkRenderPass renderPass, VkExtent2D areaSize, std::vector<VkAttachmentDescription>_attachmentDescs, std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f });

	void EndRenderPass(VkCommandBuffer cmdBuf);

	void GetNextSwapchainIndex(VkSwapchainKHR swapchain, VkSemaphore semaphore, uint32_t* swapchainIndex);

	void Present(VkSwapchainKHR swapchain, VkSemaphore semaphore, uint32_t& swapchainImageIndex);

	void CreatePipelineLayout(std::vector <VkDescriptorSetLayout> descriptorSetLayout , VkPipelineLayout& pipelineLayout);

	void DestroyPipelineLayout(VkPipelineLayout pipelineLayout);

	void CreateDescripotrPool(VkDescriptorPool& pool);

	void DestroyDescriptorPool(VkDescriptorPool pool);

	void CreateDescripotrSetLayout(VkDescriptorType type, uint32_t bindingCount ,  VkDescriptorSetLayout& descriptorSetLayout , VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

	/* width and height must be same as attachments(ImageViews) size. */
	void CreateFrameBuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, std::vector<VkImageView>attachments, VkFramebuffer& framebuffer);

	void DestroyFrameBuffer(VkFramebuffer& framebuffer);

	/* Allocate a new descriptorSet ,attention,we should be free the old or unuseful descriptorSet for save memory. */
	void AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, uint32_t newDescriptorSetCount, std::vector<VkDescriptorSet>& descriptorSet);

	void FreeDescriptorSet(VkDescriptorPool pool, std::vector<VkDescriptorSet>& descriptorSet);

	/* Image 布局转换 */
	void Transition(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspects, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1);

	void CreateSemaphore(VkSemaphore& semaphore);

	void DestroySemaphore(VkSemaphore semaphore);

	void CreateRenderSemaphores(std::vector<VkSemaphore>& semaphore);

	void DestroyRenderSemaphores(std::vector<VkSemaphore>& semaphore);

	void CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo& info , VkPipeline& pipeline);

	void CreateRenderPass(std::vector<VkAttachmentDescription>attachmentDescs, std::vector<VkSubpassDependency>subpassDependencys, std::vector<VkSubpassDescription>subpassDescs, VkRenderPass& renderPass);

	void DestroyRenderPass(VkRenderPass renderPass);

	void CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize bufferSize, VkBuffer& buffer);

	void AllocateBufferMemory(VkBuffer buffer , VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void FreeBufferMemory(VkDeviceMemory& bufferMemory);

	void DestroyBuffer(VkBuffer& buffer);

	/* 立刻序列提交,为保证运行安全,会执行一次等待运行结束 */
	void SubmitQueueImmediate(std::vector<VkCommandBuffer> cmdBufs, VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkQueue queue = VK_NULL_HANDLE);

	void SubmitQueue(std::vector<VkCommandBuffer> cmdBufs, std::vector <VkSemaphore> lastSemaphore, std::vector <VkSemaphore> newSemaphore, VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkQueue queue = VK_NULL_HANDLE);

	void SubmitQueueForPasses(std::vector<std::shared_ptr<class PassBase>> passes, VkSemaphore presentSemaphore, VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkQueue queue = VK_NULL_HANDLE);

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

	__forceinline VkDescriptorPool GetDescriptorPool()const {
		return _descriptorPool;
	}

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

	bool _bDebugEnable;

	VkDebugReportCallbackEXT			_debugReport;

	VkPhysicalDeviceProperties2			_gpuProperties{};

	VkPhysicalDeviceMemoryProperties	_gpuMemoryProperties{};

	VkPhysicalDeviceFeatures			_gpuFeatures{};

	VkPhysicalDeviceVulkan12Features	_gpuVk12Features{};

	VkQueue	_graphicsQueue;

	VkCommandPool _commandPool;

	VkDescriptorPool _descriptorPool;

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

	static std::shared_ptr<VulkanManager> _vulkanManager;

};