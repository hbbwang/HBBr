#include "VulkanWindow.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "Common.h"
#include "VulkanApp.h"

#ifdef _WIN32
#pragma comment(lib, "SDL3-static.lib")
#endif

VulkanWindow::VulkanWindow(int width, int height, const char* title)
{
	ConsoleDebug::printf_endl("Create a new vulkan window.");
	WindowHandle = SDL_CreateWindow(
		title,
		width,
		height,
		SDL_WINDOW_RESIZABLE  | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS  | 
		SDL_WINDOW_VULKAN
	);
	if (!WindowHandle)
	{
		ConsoleDebug::printf_endl_error("Create SDL3 Window failed : {}", SDL_GetError());
		return;
	}
	WindowID = SDL_GetWindowID(WindowHandle);
	//Init Swapchain
	ResetSwapchain_MainThread();
}

VulkanWindow::~VulkanWindow()
{
}

std::string VulkanWindow::GetTitle()
{
	return SDL_GetWindowTitle(WindowHandle);
}

void VulkanWindow::SetTitle(const char* title)
{
	SDL_SetWindowTitle(WindowHandle, title);
}

void VulkanWindow::SetFocus()
{
	SDL_RaiseWindow(WindowHandle);
}

void VulkanWindow::ResetSwapchain_MainThread()
{
	const auto& vkManager = VulkanManager::Get();
	//Destroy out date swapchain
	if (Swapchain)
	{
		//ConsoleDebug::printf_endl("VulkanWindow[{}] : Destroy out date swapchain.", GetTitle().c_str());
		std::vector<VkImageView>swapchainViews;
		for (auto& s: SwapchainImages)
		{
			swapchainViews.push_back(s.ImageView);
		}
		vkManager->DestroySwapchain(Swapchain, swapchainViews);
	}
	//Create Surface
	//ConsoleDebug::printf_endl("VulkanWindow[{}] : Reset vulkan surface...", GetTitle().c_str());
	vkManager->DestroySurface(Surface);
	vkManager->CreateSurface_SDL(WindowHandle, Surface);
	vkManager->GetSurfaceCapabilities(Surface, &SurfaceCapabilities);
	vkManager->CheckSurfaceFormat(Surface, SurfaceFormat);
	//ConsoleDebug::printf_endl("VulkanWindow[{}] : Create swapchain.", GetTitle().c_str());
	SurfaceSize = vkManager->CreateSwapchain(
		WindowHandle,
		{ 1,1 },
		Surface,
		SurfaceFormat,
		Swapchain,
		SwapchainImages,
		SurfaceCapabilities);
	NumSwapchainImage = (uint32_t)SwapchainImages.size();
	//Swapchain Images Layout Transition (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	VulkanApp::Get()->EnqueueRenderFunc([this]()
		{
			VulkanManager::Get()->ExecuteImmediateCommand_RenderThread(
				[this](VkCommandBuffer cmdBuf)
				{
					//Transition Swapchain Images Layout
					for (int i = 0; i < (int)SwapchainImages.size(); i++)
					{
						VulkanManager::Get()->Transition_RenderThread(cmdBuf,
							SwapchainImages[i].Image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
					}
				},
				[this]()
				{
					//ConsoleDebug::printf_endl("VulkanWindow[{}] : Transition swapchain images layout finish.", GetTitle().c_str());
					//Transition Swapchain Images Layout
					for (int i = 0; i < (int)SwapchainImages.size(); i++)
					{
						SwapchainImages[i].bIsValid = 1;
					}
				}
			);
		});
	ResetResources_MainThread();
}

void VulkanWindow::Update_MainThread()
{
	if (bNeedResetSwapchain_RenderThread)
	{
		const auto& vkManager = VulkanManager::Get();
		vkManager->DeviceWaitIdle();
		ResetSwapchain_MainThread();
		bNeedResetSwapchain_RenderThread = false;
	}
}

void VulkanWindow::Update_RenderThead()
{
	std::lock_guard<std::mutex> lock(RenderMutex);
	if (bInitialize && SwapchainImages[0].bIsValid && !bResetResources &&!bNeedResetSwapchain_RenderThread)
	{
		const auto& vkManager = VulkanManager::Get();
		//Wait for previous frame to finish.
		vkManager->WaitForFences({ ExecuteFence[CurrentFrameIndex] });
		//Get next swapchain image index.
		if (!vkManager->GetNextSwapchainIndex(Swapchain, AcquireSemaphore[CurrentFrameIndex], nullptr, &CurrentFrameIndex))
		{
			bNeedResetSwapchain_RenderThread = true;
			return;
		}
		//Begin command buffer recording.
		vkManager->BeginCommandBuffer(CmdBuf[CurrentFrameIndex]);
		//Record rendering commands here...

		//Submit command buffer to queue.
		vkManager->EndCommandBuffer(CmdBuf[CurrentFrameIndex]);
		vkManager->SubmitQueueForPasses(
			CmdBuf[CurrentFrameIndex],
			AcquireSemaphore[CurrentFrameIndex],
			QueueSemaphore[CurrentFrameIndex],
			ExecuteFence[CurrentFrameIndex]
		);
		//Present swapchain.
		if (!vkManager->Present(Swapchain, QueueSemaphore[CurrentFrameIndex], CurrentFrameIndex))
		{
			bNeedResetSwapchain_RenderThread = true;
			return;
		}
		//Get next frame index.
		CurrentFrameIndex = (CurrentFrameIndex + 1) % NumSwapchainImage;
	}
}

void VulkanWindow::ResetResources_MainThread()
{
	bResetResources = true;
	std::lock_guard<std::mutex> lock(RenderMutex);
	CurrentFrameIndex = 0;
	const auto& vkManager = VulkanManager::Get();
	{
		for (int i = 0; i < (int)ExecuteFence.size(); i++)
		{
			vkManager->DestroyFence(ExecuteFence.at(i));
			ExecuteFence.at(i) = nullptr;
		}
		ExecuteFence.resize(NumSwapchainImage);
		for (int i = 0; i < (int)NumSwapchainImage; i++)
		{
			vkManager->CreateFence(ExecuteFence.at(i));
		}
	}
	//RenderThread Resources Reset
	VulkanApp::Get()->EnqueueRenderFunc([this]()
		{
			ResetResources_RenderThread();
			bResetResources = false;
			//ConsoleDebug::printf_endl("VulkanWindow[{}] : Window reset resource finish.", GetTitle().c_str());
			if (!bInitialize)
			{
				bInitialize = true;
				ConsoleDebug::printf_endl("VulkanWindow[{}] : Window initialize finish.", GetTitle().c_str());
			}
		});
}

void VulkanWindow::ResetResources_RenderThread()
{
	const auto& vkManager = VulkanManager::Get();
	{
		//ConsoleDebug::print_endl("hBBr:Swapchain: Present Semaphore.");
		for (int i = 0; i < (int)AcquireSemaphore.size(); i++)
		{
			vkManager->DestroySemaphore(AcquireSemaphore.at(i));
			AcquireSemaphore.at(i) = nullptr;
		}
		AcquireSemaphore.resize(NumSwapchainImage);
		for (int i = 0; i < (int)NumSwapchainImage; i++)
		{
			vkManager->CreateVkSemaphore(AcquireSemaphore.at(i));
		}
	}
	{
		//ConsoleDebug::print_endl("hBBr:Swapchain: Present Semaphore.");
		for (int i = 0; i < (int)QueueSemaphore.size(); i++)
		{
			vkManager->DestroySemaphore(QueueSemaphore.at(i));
			QueueSemaphore.at(i) = nullptr;
		}
		QueueSemaphore.resize(NumSwapchainImage);
		for (int i = 0; i < (int)NumSwapchainImage; i++)
		{
			vkManager->CreateVkSemaphore(QueueSemaphore.at(i));
		}
	}
	if (CmdBuf.size() != NumSwapchainImage)
	{
		//ConsoleDebug::print_endl("hBBr:Swapchain: Allocate Main CommandBuffers.");
		for (int i = 0; i < (int)CmdBuf.size(); i++)
		{
			vkManager->FreeCommandBuffer(vkManager->GetCommandPool(), CmdBuf.at(i));
			CmdBuf.at(i) = nullptr;
		}
		CmdBuf.resize(NumSwapchainImage);
		for (int i = 0; i < (int)NumSwapchainImage; i++)
		{
			vkManager->AllocateCommandBuffer(vkManager->GetCommandPool(), CmdBuf.at(i));
		}
	}
}