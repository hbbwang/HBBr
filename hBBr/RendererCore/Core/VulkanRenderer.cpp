#include "VulkanRenderer.h"
#include "Texture.h"
#include "RendererConfig.h"
#include "ConsoleDebug.h"
VulkanManager* VulkanRenderer::_vulkanManager = NULL;
uint32_t  VulkanRenderer::_currentFrameIndex = 0;

VulkanRenderer::VulkanRenderer(void* windowHandle , bool bDebug)
{
	ConsoleDebug::print_endl(RendererLauguage::GetText("T000000"));
	bRendererRelease = false;
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
	_vulkanManager->CreateRenderSemaphores(_presentSemaphore);
	_vulkanManager->CheckSurfaceFormat(_surface , _surfaceFormat);
	_surfaceSize = _vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews);
}

VulkanRenderer::~VulkanRenderer()
{
	bRendererRelease = true;
	if (_vulkanManager)
	{
		vkDeviceWaitIdle(_vulkanManager->GetDevice());
		_vulkanManager->DestroyCommandPool();
		_vulkanManager->DestroySwapchain(_swapchain, _swapchainImages, _swapchainImageViews);
		_vulkanManager->DestroyRenderSemaphores(_presentSemaphore);
		_vulkanManager->DestroySurface(_surface);
		delete _vulkanManager;
		_vulkanManager = NULL;
	}
}

void VulkanRenderer::Render()
{
	if (_vulkanManager && !bRendererRelease)
	{
		CheckSwapchainOutOfData();

		//what swapchain index need to present?
		uint32_t swapchainIndex;
		_vulkanManager->GetNextSwapchainIndex(_swapchain, _presentSemaphore[_currentFrameIndex], &swapchainIndex);

		//Present swapchain.
		_vulkanManager->Present(_swapchain, _presentSemaphore[_currentFrameIndex], swapchainIndex);

		//Get next frame index.
		_currentFrameIndex = (_currentFrameIndex + 1) % _vulkanManager->GetSwapchainBufferCount();
	}
}

void VulkanRenderer::CheckSwapchainOutOfData()
{
	if (_vulkanManager)
	{
		if (_surfaceSize.width == _windowSize.width && _surfaceSize.height == _windowSize.height)
		{
			return;
		}
		vkDeviceWaitIdle(_vulkanManager->GetDevice());
		_vulkanManager->DestroySwapchain(_swapchain, _swapchainImages, _swapchainImageViews);
		_surfaceSize = _vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews);
	}
}

void VulkanRenderer::ResetWindowSize(uint32_t width, uint32_t height)
{
	_windowSize.width = width;
	_windowSize.height = height;
}
