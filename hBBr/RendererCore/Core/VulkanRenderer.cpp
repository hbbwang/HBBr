#include "VulkanRenderer.h"
#include "Texture.h"
#include "RendererConfig.h"
#include "ConsoleDebug.h"
#include "PassManager.h"
#include "PassBase.h"
#include "FileSystem.h"
#include "Shader.h"
#include "Thread.h"
#if IS_EDITOR
#include "ShaderCompiler.h"
#endif

std::map<HString, VulkanRenderer*>		VulkanRenderer::_renderers;
uint32_t								VulkanRenderer::_currentFrameIndex;

VulkanRenderer::VulkanRenderer(void* windowHandle, const char* rendererName)
{
	_currentFrameIndex = 0;
	_bRendererRelease = false;
	_bInit = false;
	_swapchain = VK_NULL_HANDLE;
	_surface = VK_NULL_HANDLE;
	_windowHandle = windowHandle;
	_rendererName = rendererName;
	Init();
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::Release()
{
	_bRendererRelease = true;
	VulkanManager* _vulkanManager = VulkanManager::GetManager();
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	_passManager.reset();
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), _cmdBuf);
	_vulkanManager->DestroySwapchain(_swapchain, _swapchainImageViews);
	_vulkanManager->DestroyRenderSemaphores(_presentSemaphore);
	_vulkanManager->DestroyRenderSemaphores(_queueSubmitSemaphore);
	_vulkanManager->DestroySurface(_surface);
	VulkanRenderer::_renderers.erase(GetName());
	delete this;
}

void VulkanRenderer::Init()
{	
	VulkanManager* _vulkanManager = VulkanManager::GetManager();
	//Surface
	_vulkanManager->CreateSurface(_windowHandle, _surface);
	//Swapchain
	_vulkanManager->CreateRenderSemaphores(_presentSemaphore);
	_vulkanManager->CreateRenderSemaphores(_queueSubmitSemaphore);
	_vulkanManager->CheckSurfaceFormat(_surface, _surfaceFormat);
	_surfaceSize = _vulkanManager->CreateSwapchain(_windowSize, _surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews);
	//CommandBuffer
	_cmdBuf.resize(_vulkanManager->GetSwapchainBufferCount());
	for (int i = 0; i < _cmdBuf.size(); i++)
	{
		VulkanManager::GetManager()->AllocateCommandBuffer(VulkanManager::GetManager()->GetCommandPool(), _cmdBuf[i]);
	}
	//Set renderer map , Add new renderer
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	auto rendererName = _rendererName;
	for (int nameIndex = -1;;)
	{
		auto it = _renderers.find(_rendererName);
		if (it == _renderers.end())
		{
			_renderers.emplace(std::make_pair(_rendererName, this));
			break;
		}
		else
		{
			nameIndex++;
			_rendererName = HString(rendererName) + "_" + HString::FromInt(nameIndex);
			MessageOut((HString("Has the same name of renderer.Random a new name is [") + _rendererName + "]").c_str(), false, true);
		}
	}
	//Init passes
	_passManager.reset(new PassManager());
	_passManager->PassesInit(this);
	_bInit = true;
}

void VulkanRenderer::Render()
{
	if (!_bRendererRelease && _bInit)
	{
		_frameRate = _frameTime.FrameRate_ms();

		if (!Resizing())
			return;

		VulkanManager* _vulkanManager = VulkanManager::GetManager();

		uint32_t _swapchainIndex = 0;

		//Which swapchain index need to present?
		if (!_vulkanManager->GetNextSwapchainIndex(_swapchain, _presentSemaphore[_currentFrameIndex], &_swapchainIndex))
		{
			Resizing(true);
		}	
		_passManager->PassesUpdate();
		//Present swapchain.
		if (!_vulkanManager->Present(_swapchain, _queueSubmitSemaphore[_currentFrameIndex], _swapchainIndex))
		{
			Resizing(true);
		}

		//Get next frame index.
		_currentFrameIndex = (_currentFrameIndex + 1) % _vulkanManager->GetSwapchainBufferCount();
	}
}

void VulkanRenderer::RendererResize(uint32_t w, uint32_t h)
{
	_bResize = true;
	_windowSize.width = w;
	_windowSize.height = h;
}

bool VulkanRenderer::Resizing(bool bForce)
{
	if (!_bRendererRelease)
	{
		VulkanManager* _vulkanManager = VulkanManager::GetManager();
		if (_bResize || bForce || _swapchain == VK_NULL_HANDLE )
		{
			vkDeviceWaitIdle(_vulkanManager->GetDevice());
			_vulkanManager->DestroySwapchain(_swapchain, _swapchainImageViews);
			_surfaceSize = _vulkanManager->CreateSwapchain(_windowSize, _surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews);
			if (_swapchain == VK_NULL_HANDLE)
			{
				return false;
			}
			_passManager->PassesReset();
			_bResize = false;
		}
	}
	return true;
}
