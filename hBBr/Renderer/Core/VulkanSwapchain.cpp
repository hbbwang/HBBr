#include "../Common/Common.h"
#include "VulkanSwapchain.h"
#include "VulkanWindow.h"

VulkanSwapchain::VulkanSwapchain(SDL_Window* window)
{
	ConsoleDebug::print_endl("hBBr:Start Init Main VulkanSwapchain...");
	WindowHandle = window;
	const auto& vkManager = VulkanManager::Get();
	//Surface
	vkManager->ReCreateSurface_SDL(WindowHandle, Surface);
	vkManager->GetSurfaceCapabilities(Surface, &SurfaceCapabilities);
	//Swapchain
	vkManager->CheckSurfaceFormat(Surface, SurfaceFormat);
	ConsoleDebug::print_endl("hBBr:Start Create Swapchain.");
	SurfaceSize = vkManager->CreateSwapchain(
		_form->window,
		{ 1,1 },
		_surface,
		_surfaceFormat,
		_swapchain,
		_swapchainImages,
		_swapchainImageViews,
		_surfaceCapabilities);
}

VulkanSwapchain::~VulkanSwapchain()
{
}

void VulkanSwapchain::Update_MainThread()
{
	if(!bIsInitialized)
		bIsInitialized = true;

}

void VulkanSwapchain::Update_RenderThread()
{
}
