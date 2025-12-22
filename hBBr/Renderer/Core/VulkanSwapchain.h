#pragma once
#include <thread>
#include <memory>
#include <mutex>
#include "VulkanManager.h"

class VulkanSwapchain
{
	friend class VulkanApp;
	friend class VulkanWindow;
public:
	VulkanSwapchain(SDL_Window* window);
	~VulkanSwapchain();
protected:
	//Main Thread Objects
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	VkSurfaceFormatKHR SurfaceFormat{};
	VkSurfaceCapabilitiesKHR SurfaceCapabilities{};
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	VkExtent2D SurfaceSize{};
	std::vector<VulkanImage> SwapchainImages;
	uint32_t NumSwapchainImage = 0;
	uint32_t CurrentFrameIndex = 0;
	std::vector<VkFence> ExecuteFence;
	//Render Thread Objects
	std::vector<VkSemaphore> QueueSemaphore;
	std::vector<VkSemaphore> AcquireSemaphore;
	std::vector<VkCommandBuffer> CmdBuf;
	//Functions
	void Update_MainThread();
	void Update_RenderThread();
	void ResetResources_MainThread();
	void ResetResources_RenderThread();
private:
	SDL_Window* WindowHandle = nullptr;
	std::mutex RenderMutex;
	bool bResetResources = false;
	bool bInitialize = false;
};

//std::memory_order_relaxed：最宽松的模式。只保证当前原子操作是原子的，不提供任何同步或顺序约束。适用于计数器等无需同步其他内存操作的场景。
//std::memory_order_consume：较弱的依赖顺序。仅限制依赖于该原子值的读写不能重排到此操作之前。实际使用受限，多数编译器将其视为 acquire 处理。
//std::memory_order_acquire：用于读操作（如 load）。保证该操作之后的所有读写不会被重排到它前面。常用于获取锁或读取共享数据前的同步。
//std::memory_order_release：用于写操作（如 store）。保证该操作之前的所有读写不会被重排到它后面。通常配合 acquire 使用，实现线程间的数据发布。
//std::memory_order_acq_rel：同时具有 acquire 和 release 语义，适用于读 - 修改 - 写操作（如 fetch_add、compare_exchange）。既防止前面的读写被后移，也防止后面的读写被前移。
//std::memory_order_seq_cst：最强的一致性模型，默认选项。除了具备 acq_rel 的所有特性外，还保证所有线程看到的操作顺序一致。性能开销最大，但逻辑最直观。