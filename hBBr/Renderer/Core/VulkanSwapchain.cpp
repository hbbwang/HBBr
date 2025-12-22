#include "../Common/Common.h"
#include "VulkanSwapchain.h"
#include "VulkanWindow.h"
#include "VulkanApp.h"

VulkanSwapchain::VulkanSwapchain(SDL_Window* window)
{
	WindowHandle = window;
	ConsoleDebug::print_endl("hBBr:Start Init Main VulkanSwapchain...");
	const auto& vkManager = VulkanManager::Get();
	//Surface
	vkManager->ReCreateSurface_SDL(WindowHandle, Surface);
	vkManager->GetSurfaceCapabilities(Surface, &SurfaceCapabilities);
	//Swapchain
	vkManager->CheckSurfaceFormat(Surface, SurfaceFormat);
	ConsoleDebug::print_endl("hBBr:Start Create Swapchain.");
	SurfaceSize = vkManager->CreateSwapchain(
		WindowHandle,
		{ 1,1 },
		Surface,
		SurfaceFormat,
		Swapchain,
		SwapchainImages,
		SurfaceCapabilities);
	NumSwapchainImage = SwapchainImages.size();
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

VulkanSwapchain::~VulkanSwapchain()
{
}

void VulkanSwapchain::Update_MainThread()
{

}

void VulkanSwapchain::Update_RenderThread()
{
	std::lock_guard<std::mutex> lock(RenderMutex);
	if (bInitialize && !bResetResources)
	{
		const auto& vkManager = VulkanManager::Get();
		//Get next swapchain image index.
		if (!vkManager->GetNextSwapchainIndex(Swapchain, AcquireSemaphore[CurrentFrameIndex], nullptr, &CurrentFrameIndex))
		{
			return;
		}
		//Present swapchain.
		if (!vkManager->Present(Swapchain, QueueSemaphore[CurrentFrameIndex], CurrentFrameIndex))
		{
			return;
		}
		//Get next frame index.
		CurrentFrameIndex = (CurrentFrameIndex + 1) % NumSwapchainImage;
	}
}

void VulkanSwapchain::ResetResources_MainThread()
{
	bResetResources = true;
	std::lock_guard<std::mutex> lock(RenderMutex);
	const auto& vkManager = VulkanManager::Get();
	if (ExecuteFence.size() != NumSwapchainImage)
	{
		//ConsoleDebug::print_endl("hBBr:Swapchain: image acquired fences.");
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
			bInitialize = true;
		});
}

void VulkanSwapchain::ResetResources_RenderThread()
{
	const auto& vkManager = VulkanManager::Get();
	if (AcquireSemaphore.size() != NumSwapchainImage)
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
	if (QueueSemaphore.size() != NumSwapchainImage)
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
