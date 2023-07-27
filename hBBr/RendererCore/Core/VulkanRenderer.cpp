#include "VulkanRenderer.h"
#include "Texture.h"
#include "RendererConfig.h"
#include "ConsoleDebug.h"
#include "PassManager.h"
#include "PassBase.h"
#include "FileSystem.h"
#include "Shader.h"
#if IS_EDITOR
#include "ShaderCompiler.h"
#endif

std::map<HString, VulkanRenderer*>		VulkanRenderer::_renderers;
uint32_t								VulkanRenderer::_currentFrameIndex;

VulkanRenderer::VulkanRenderer(void* windowHandle, const char* rendererName)
{
	VulkanManager* _vulkanManager = VulkanManager::GetManager();
	ConsoleDebug::print_endl(RendererLauguage::GetText("T000000"));
	_currentFrameIndex = 0;
	_swapchainIndex = 0;
	_bRendererRelease = false;
	_bInit = false;
	_swapchain = VK_NULL_HANDLE;
	_surface = VK_NULL_HANDLE;
	//Surface
	_vulkanManager->CreateSurface(windowHandle, _surface);
	//Swapchain
	_vulkanManager->CreateRenderSemaphores(_presentSemaphore);
	_vulkanManager->CheckSurfaceFormat(_surface , _surfaceFormat);
	_surfaceSize = _vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain, _swapchainTextures , _swapchainImageViews);
	//Set renderer map , Add new renderer
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	_rendererName = rendererName;
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
			MessageOut((HString("Has the same name of renderer.Random a new name is [") +_rendererName + "]").c_str(), false, true);
		}
	}
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
	_vulkanManager->DestroySwapchain(_swapchain, _swapchainTextures);
	_vulkanManager->DestroyRenderSemaphores(_presentSemaphore);
	_vulkanManager->DestroySurface(_surface);
	VulkanRenderer::_renderers.erase(GetName());
	delete this;
}

void VulkanRenderer::Init()
{	
	_bInit = true;
	//Init passes
	_passManager.reset(new PassManager());
	_passManager->PassesInit(this);
}

void VulkanRenderer::Render()
{
	VulkanManager* _vulkanManager = VulkanManager::GetManager();
	
	//Which swapchain index need to present?
	_vulkanManager->GetNextSwapchainIndex(_swapchain, _presentSemaphore[_currentFrameIndex], &_swapchainIndex);

	_passManager->PassesUpdate();

	//Present swapchain.
	_vulkanManager->Present(_swapchain, *_passManager->GetTheLastSemaphore(), _swapchainIndex);

	//Get next frame index.
	_currentFrameIndex = (_currentFrameIndex + 1) % _vulkanManager->GetSwapchainBufferCount();
}

void VulkanRenderer::RendererResize()
{
	_bResize = true;
	VulkanManager* _vulkanManager = VulkanManager::GetManager();
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	_vulkanManager->DestroySwapchain(_swapchain, _swapchainTextures);
	_surfaceSize = _vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain, _swapchainTextures, _swapchainImageViews);
	_bResize = false;
}

