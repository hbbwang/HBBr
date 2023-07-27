#pragma once
//Vulkan底层核心管理类

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
	#define VK_USE_PLATFORM_WIN32_KHR  1 //support win32
	#include <stdlib.h>
	#include <Windows.h>
#elif defined(__ANDROID__)
	#include <vulkan/vulkan_android.h>
#elif defined(__linux__)
	#include <vulkan/vulkan_xcb.h>
#endif

#include "../Common/Common.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <array>
#include "HString.h"

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

	HBBR_API __forceinline static VulkanManager* GetManager() {
		return _vulkanManager.get();
	}

	HBBR_API static void ReleaseManager() {
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
	VkExtent2D GetSurfaceSize(VkExtent2D windowSizeIn, VkSurfaceKHR surface);

	/* 检查Surface支持 */
	void CheckSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR& surfaceFormat);

	/* 创建Swapchain */
	VkExtent2D CreateSwapchain(VkExtent2D surfaceSize, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSwapchainKHR& newSwapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews);

	/* 创建Swapchain From Texture Class */
	VkExtent2D CreateSwapchain(VkExtent2D surfaceSize, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkSwapchainKHR& newSwapchain, std::vector<std::shared_ptr<class Texture>>& textures , std::vector<VkImageView>& swapchainImageViews);

	/* 释放Swapchain */
	void DestroySwapchain(VkSwapchainKHR& swapchain, std::vector<VkImageView>& swapchainImageViews);

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
	void CreateFrameBuffers(VkExtent2D FrameBufferSize, VkRenderPass renderPass, std::vector<VkImageView> attachments, std::vector<VkFramebuffer>& frameBuffers);

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

	/* return the swapchain is normal (not out of data). */
	bool GetNextSwapchainIndex(VkSwapchainKHR swapchain, VkSemaphore semaphore, uint32_t* swapchainIndex);

	void Present(VkSwapchainKHR swapchain, VkSemaphore semaphore, uint32_t& swapchainImageIndex);

	void CreatePipelineLayout(std::vector <VkDescriptorSetLayout> descriptorSetLayout , VkPipelineLayout& pipelineLayout);

	void DestroyPipelineLayout(VkPipelineLayout& pipelineLayout);

	void CreateDescripotrPool(VkDescriptorPool& pool);

	void DestroyDescriptorPool(VkDescriptorPool& pool);

	void CreateDescripotrSetLayout(VkDescriptorType type, uint32_t bindingCount ,  VkDescriptorSetLayout& descriptorSetLayout , VkShaderStageFlags shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

	/* width and height must be same as attachments(ImageViews) size. */
	void CreateFrameBuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, std::vector<VkImageView>attachments, VkFramebuffer& framebuffer);

	void DestroyFrameBuffer(VkFramebuffer& framebuffer);

	/* Allocate a new descriptorSet ,attention,we should be free the old or unuseful descriptorSet for save memory. */
	void AllocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, uint32_t newDescriptorSetCount, std::vector<VkDescriptorSet>& descriptorSet);

	/* Pool must created setting VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT */
	void FreeDescriptorSet(VkDescriptorPool pool, std::vector<VkDescriptorSet>& descriptorSet);

	/* Image 布局转换 */
	void Transition(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspects, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1);

	void CreateSemaphore(VkSemaphore& semaphore);

	void DestroySemaphore(VkSemaphore& semaphore);

	void CreateRenderSemaphores(std::vector<VkSemaphore>& semaphore);

	void DestroyRenderSemaphores(std::vector<VkSemaphore>& semaphore);

	void CreateFence(VkFence& fence);

	void DestroyFence(VkFence& fence);

	void CreateRenderFences(std::vector<VkFence>& fences);

	void DestroyRenderFences(std::vector<VkFence>& fences);

	void WaitForFences(std::vector<VkFence> fences, bool bReset = true, uint64_t timeOut = UINT64_MAX);

	void WaitSemaphores(std::vector<VkSemaphore> semaphores , uint64_t timeOut = UINT64_MAX);

	void CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo& info , VkPipeline& pipeline);

	void CreateRenderPass(std::vector<VkAttachmentDescription>attachmentDescs, std::vector<VkSubpassDependency>subpassDependencys, std::vector<VkSubpassDescription>subpassDescs, VkRenderPass& renderPass);

	void DestroyRenderPass(VkRenderPass& renderPass);

	void CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize bufferSize, VkBuffer& buffer);

	void AllocateBufferMemory(VkBuffer buffer , VkDeviceMemory& bufferMemory, VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void FreeBufferMemory(VkDeviceMemory& bufferMemory);

	void DestroyBuffer(VkBuffer& buffer);

	void CreateShaderModule(std::vector<char> data , VkShaderModule& shaderModule);

	void CreateShaderModule(VkDevice device, std::vector<char> data, VkShaderModule& shaderModule);

	/* 立刻序列提交,为保证运行安全,会执行一次等待运行结束 */
	void SubmitQueueImmediate(std::vector<VkCommandBuffer> cmdBufs, VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkQueue queue = VK_NULL_HANDLE);

	void SubmitQueue(std::vector<VkCommandBuffer> cmdBufs, std::vector <VkSemaphore> lastSemaphore, std::vector <VkSemaphore> newSemaphore, VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkQueue queue = VK_NULL_HANDLE);

	VkViewport GetViewport(float w,float h);

	void SubmitQueueForPasses(VkCommandBuffer cmdBuf,std::vector<std::shared_ptr<class PassBase>> passes, VkSemaphore presentSemaphore, VkSemaphore submitFinishSemaphore, VkFence executeFence , VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkQueue queue = VK_NULL_HANDLE);

	/* CMD */
	void CmdSetViewport(VkCommandBuffer cmdbuf, std::vector<VkExtent2D> viewports);
	/*-----------------*/

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

	static std::unique_ptr<VulkanManager> _vulkanManager;

};