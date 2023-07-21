#include "VulkanRenderer.h"

VulkanManager* VulkanRenderer::_vulkanManager = NULL;


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
	//Swapchain
	_vulkanManager->CheckSurfaceSupport(_surface , _surfaceFormat);
	_vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain);
}

VulkanRenderer::~VulkanRenderer()
{
	if (_vulkanManager)
	{
		_vulkanManager->DestroySwapchain(_swapchain);
		_vulkanManager->DestroySurface(_surface);
		//
		delete _vulkanManager;
		_vulkanManager = NULL;
	}
}
