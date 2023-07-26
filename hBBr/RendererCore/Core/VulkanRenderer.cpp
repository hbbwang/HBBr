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

std::unique_ptr<std::thread>		VulkanRenderer::_renderThread;
std::map<HString, VulkanRenderer*>	VulkanRenderer::_renderers;
uint32_t							VulkanRenderer::_currentFrameIndex;

std::mutex renderThreadMutex;

#if IS_EDITOR

void EditorContentInit()
{
	Shaderc::ShaderCompiler::CompileAllShaders(FileSystem::GetShaderIncludeAbsPath().c_str());
}

#endif

static void RenderThreadUpdate()
{
	//Editor init
#if IS_EDITOR
	EditorContentInit();
#endif
	Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());
	while (VulkanRenderer::_renderers.size() > 0 )
	{
		for (auto i : VulkanRenderer::_renderers)
		{
			if (i.second->IsRendererWantRelease())
			{
				VulkanRenderer::_renderers.erase(i.second->GetName());
				break;
			}
			if (!i.second->IsInit())
			{
				i.second->Init();
			}
			else
			{
				i.second->Render();
			}
		}
	}
}

VulkanRenderer::VulkanRenderer(void* windowHandle, const char* rendererName)
{
	const std::shared_ptr<VulkanManager> _vulkanManager = VulkanManager::GetManager();
	ConsoleDebug::print_endl(RendererLauguage::GetText("T000000"));
	_currentFrameIndex = 0;
	_swapchainIndex = 0;
	_bRendererRelease = false;
	_bRendererResize = false;
	_bRendering = false;
	_bInit = false;
	_swapchain = VK_NULL_HANDLE;
	_surface = VK_NULL_HANDLE;
	//Surface
	_vulkanManager->CreateSurface(windowHandle, _surface);
	//Swapchain
	_vulkanManager->CreateRenderSemaphores(_presentSemaphore);
	_vulkanManager->CheckSurfaceFormat(_surface , _surfaceFormat);
	_surfaceSize = _vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain, _swapchainTextures);

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
			MessageOut((HString("Has the same name of renderer.Random a new name is [") + _rendererName + "]").c_str(), false, true);
		}
	}

	//Create render thread.
	if (_renderThread == NULL)
		_renderThread.reset(new std::thread(RenderThreadUpdate));
}

VulkanRenderer::~VulkanRenderer()
{
	const std::shared_ptr<VulkanManager> _vulkanManager = VulkanManager::GetManager();
	_bRendererRelease = true;
	if (_vulkanManager)
	{
		vkDeviceWaitIdle(_vulkanManager->GetDevice());
		_passManager.reset();
		_vulkanManager->DestroySwapchain(_swapchain, _swapchainTextures);
		_vulkanManager->DestroyRenderSemaphores(_presentSemaphore);
		_vulkanManager->DestroySurface(_surface);
	}
	if (_renderers.size() == 1 )//最后一个,等待渲染结束
	{
		//Wait for thread stop.
		_renderThread->join();
		_vulkanManager->ReleaseManager();
	}
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
	const std::shared_ptr<VulkanManager> _vulkanManager = VulkanManager::GetManager();
	if (!_bRendererResize)
	{
		_bRendering = true;

		std::lock_guard<std::mutex> renderThreadLock(renderThreadMutex);

		//Check swapchain valid.
		CheckSwapchainOutOfData();

		//Which swapchain index need to present?
		_vulkanManager->GetNextSwapchainIndex(_swapchain, _presentSemaphore[_currentFrameIndex], &_swapchainIndex);

		//_passManager->PassesUpdate();

		//Present swapchain.
		_vulkanManager->Present(_swapchain, _presentSemaphore[_currentFrameIndex], _swapchainIndex);

		//Get next frame index.
		_currentFrameIndex = (_currentFrameIndex + 1) % _vulkanManager->GetSwapchainBufferCount();

		_bRendering = false;
	}
}

void VulkanRenderer::ResetWindowSize(uint32_t width, uint32_t height)
{
	const std::shared_ptr<VulkanManager> _vulkanManager = VulkanManager::GetManager();
	_bRendererResize = true;
	if (_vulkanManager && !_bRendererRelease)
	{
		//Wait render setting finish.
		std::lock_guard<std::mutex> renderThreadLock(renderThreadMutex);
		//Resize
		_windowSize.width = width;
		_windowSize.height = height;
		RendererResize();
	}
	_bRendererResize = false;
}

void VulkanRenderer::CheckSwapchainOutOfData()
{
	const std::shared_ptr<VulkanManager> _vulkanManager = VulkanManager::GetManager();
	if (_vulkanManager)
	{
		if (_surfaceSize.width == _windowSize.width && _surfaceSize.height == _windowSize.height)
		{
			return;
		}
		RendererResize();
	}
}

void VulkanRenderer::RendererResize()
{
	const std::shared_ptr<VulkanManager> _vulkanManager = VulkanManager::GetManager();
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	_vulkanManager->DestroySwapchain(_swapchain, _swapchainTextures);
	_surfaceSize = _vulkanManager->CreateSwapchain(_surface, _surfaceFormat, _swapchain, _swapchainTextures);
}

