#include "VulkanRenderer.h"
#include "Texture.h"

VulkanManager* VulkanRenderer::_vulkanManager = NULL;
uint32_t  VulkanRenderer::_swapchainBufferIndex = 0;

VulkanRenderer::VulkanRenderer(void* windowHandle , bool bDebug)
{
	_swapchain = VK_NULL_HANDLE;
	_surface = VK_NULL_HANDLE;

	if(_vulkanManager==NULL)
		_vulkanManager = (new VulkanManager());
	
	//Init
	_vulkanManager->InitInstance(bDebug);
	_vulkanManager->CreateSurface(windowHandle, _surface);
	_vulkanManager->InitDevice(_surface);
	_vulkanManager->InitDebug();
	_vulkanManager->CreateCommandPool();
	//Swapchain
	_vulkanManager->CheckSurfaceFormat(_surface , _surfaceFormat);
	_surfaceSize = _vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews);
}

VulkanRenderer::~VulkanRenderer()
{
	if (_vulkanManager)
	{
		vkDeviceWaitIdle(_vulkanManager->GetDevice());
		_vulkanManager->DestroyCommandPool();
		_vulkanManager->DestroySwapchain(_swapchain, _swapchainImages, _swapchainImageViews);
		_vulkanManager->DestroySurface(_surface);
		//
		delete _vulkanManager;
		_vulkanManager = NULL;
	}
}

void VulkanRenderer::Render()
{
	_vulkanManager->GetNextSwapchainIndex(_swapchain, _swapchainBufferIndex);
	_vulkanManager->Present(_swapchain, _swapchainBufferIndex);
}
