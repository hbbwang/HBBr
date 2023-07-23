#include "VulkanRenderer.h"
#include "Texture.h"
#include "RendererConfig.h"
#include "ConsoleDebug.h"
#include "PassManager.h"

VulkanManager* VulkanRenderer::_vulkanManager = NULL;
uint32_t  VulkanRenderer::_currentFrameIndex = 0;

static void RenderThreadUpdate(VulkanRenderer* renderer)
{
	while (!renderer->IsRendererWantRelease() && !renderer->IsRendererWantResize())
	{
		renderer->Render();
	}
}

VulkanRenderer::VulkanRenderer(void* windowHandle , bool bDebug)
{
	ConsoleDebug::print_endl(RendererLauguage::GetText("T000000"));
	_bRendererRelease = false;
	_bRendererResize = false;
	_bRendering = false;
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
	//CommandBuffers
	_commandBuffers.resize(_vulkanManager->GetSwapchainBufferCount());
	for (int i = 0; i < _commandBuffers.size(); i++)
	{
		_vulkanManager->CreateCommandBuffer(_vulkanManager->GetCommandPool(), _commandBuffers[i]);
	}
	//Init passes
	_passManager.reset(new PassManager());
	_passManager->PassesInit();
	//Create render thread.
	_renderThread = std::thread(RenderThreadUpdate, this);
}

VulkanRenderer::~VulkanRenderer()
{
	_bRendererRelease = true;
	//Wait for thread stop.
	_renderThread.join();
	if (_vulkanManager)
	{
		vkDeviceWaitIdle(_vulkanManager->GetDevice());
		_vulkanManager->FreeCommandBuffers(_vulkanManager->GetCommandPool(), _commandBuffers);
		_passManager->PassesRelease();
		_passManager.reset();
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
	_bRendering = true;

	//Check swapchain invail.
	CheckSwapchainOutOfData();

	//Which swapchain index need to present?
	uint32_t swapchainIndex;
	_vulkanManager->GetNextSwapchainIndex(_swapchain, _presentSemaphore[_currentFrameIndex], &swapchainIndex);

	_passManager->PassesUpdate();

	//Present swapchain.
	_vulkanManager->Present(_swapchain, _presentSemaphore[_currentFrameIndex], swapchainIndex);

	//Get next frame index.
	_currentFrameIndex = (_currentFrameIndex + 1) % _vulkanManager->GetSwapchainBufferCount();

	_bRendering = false;
}

void VulkanRenderer::CheckSwapchainOutOfData()
{
	if (_vulkanManager)
	{
		if (_surfaceSize.width == _windowSize.width && _surfaceSize.height == _windowSize.height)
		{
			return;
		}
		RendererResize();
	}
}

void VulkanRenderer::ResetWindowSize(uint32_t width, uint32_t height)
{
	_bRendererResize = true;
	//Wait render setting finish.
	while (_bRendering) { _Sleep(1); }
	//Resize
	_windowSize.width = width;
	_windowSize.height = height;
	RendererResize();
}

void VulkanRenderer::RendererResize()
{
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	_vulkanManager->DestroySwapchain(_swapchain, _swapchainImages, _swapchainImageViews);
	_surfaceSize = _vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews);
}
