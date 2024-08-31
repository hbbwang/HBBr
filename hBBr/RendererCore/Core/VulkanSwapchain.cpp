#include "VulkanSwapchain.h"
#include "VulkanRenderer.h"
#include "Texture2D.h"
#include "RendererConfig.h"
#include "ConsoleDebug.h"
#include "FileSystem.h"
#include "Shader.h"
#include "Thread.h"
#include "Asset/World.h"
#include "Component/GameObject.h"
#include "Component/CameraComponent.h"
#include "Pass/PassManager.h"
#include "Pass/PassBase.h"
#include "Pass/BasePass.h"
#include "Pass/ImguiPass.h"
#include "Pass/ImguiPassEditor.h"
#if IS_EDITOR
#include "ShaderCompiler.h"
#endif

VulkanSwapchain::VulkanSwapchain(SDL_Window* windowHandle)
{
	_windowHandle = windowHandle;
	_vulkanManager = VulkanManager::GetManager();
	//Surface
	ConsoleDebug::print_endl("hBBr:Start init Surface.");
	_vulkanManager->ReCreateSurface_SDL(_windowHandle, _surface);
	_vulkanManager->GetSurfaceCapabilities(_surface, &_surfaceCapabilities);

	//Swapchain
	ConsoleDebug::print_endl("hBBr:Start Check Surface Format.");
	_vulkanManager->CheckSurfaceFormat(_surface, _surfaceFormat);
	ConsoleDebug::print_endl("hBBr:Start Create Swapchain.");
	_surfaceSize = _vulkanManager->CreateSwapchain(
		_windowHandle,
		{ 1,1 },
		_surface,
		_surfaceFormat,
		_swapchain,
		_swapchainImages,
		_swapchainImageViews,
		_surfaceCapabilities);

	//Set renderer map , Add new renderer
	vkDeviceWaitIdle(_vulkanManager->GetDevice());

	//auto mainRenderer = CreateRenderer("MainRenderer");
	//mainRenderer->_renderSize = _surfaceSize;

	ResetResource();

}

VulkanSwapchain::~VulkanSwapchain()
{
}

VulkanRenderer* VulkanSwapchain::CreateRenderer(HString rendererName)
{
	VulkanRenderer* newRenderer = nullptr;
	newRenderer = new VulkanRenderer(this, rendererName.c_str());

	if (_renderers.size() <= 0)
	{
		newRenderer->_bIsMainRenderer = true;
	}

	for (int nameIndex = -1;;)
	{
		auto it = _renderers.find(rendererName);
		if (it == _renderers.end())
		{
			_renderers.emplace(std::make_pair(rendererName, newRenderer));
			break;
		}
		else
		{
			nameIndex++;
			rendererName = HString(rendererName) + "_" + HString::FromInt(nameIndex);
			MessageOut((HString("Has the same name of renderer.Random a new name is [") + rendererName + "]"), false, false);
		}
	}
	ConsoleDebug::print_endl(HString("Create Renderer : ") + rendererName);

	newRenderer->_renderSize = _surfaceSize;

	return newRenderer;
}

void VulkanSwapchain::DestroyRenderer(HString rendererName)
{
	auto it = _renderers.find(rendererName);
	if (it != _renderers.end())
	{
		DestroyRenderer(it->second);
	}
}

void VulkanSwapchain::DestroyRenderer(VulkanRenderer* renderer)
{
	if (renderer != nullptr)
	{
		for (auto& i: _renderers)
		{
			if (i.second == renderer)
			{
				renderer->Release();
				_renderers.erase(i.first);
				return;
			}
		}
	}
}

void VulkanSwapchain::Release()
{
	for (auto& i : _renderers)
	{
		i.second->Release();
	}
	_renderers.clear();
#if IS_EDITOR
	_imguiPassEditor.reset();
#endif
	_vulkanManager->FreeCommandBuffers(_vulkanManager->GetCommandPool(), _cmdBuf);
	_vulkanManager->DestroySwapchain(_swapchain, _swapchainImageViews);
	//_vulkanManager->DestroyRenderFences(_imageAcquiredFences);
	_vulkanManager->DestroySurface(_surface);
	_vulkanManager->DestroyRenderSemaphores(_acquireSemaphore);
	_vulkanManager->DestroyRenderSemaphores(_queueSemaphore);
	_vulkanManager->DestroyRenderFences(_executeFence);
	delete this;
}

void VulkanSwapchain::Update()
{
	if (!bInit)
	{
		bInit = true;
		#if IS_EDITOR
		if (!_imguiPassEditor)
		{
			_imguiPassEditor.reset(new ImguiPassEditor(this));
			_imguiPassEditor->SetPassName("Editor GUI Pass");
			_imguiPassEditor->PassInit();
		}
		#endif
	}
	else
	{
		//这个 _swapchainIndex 和 _currentFrameIndex 不是一个东西，前者是有效交换链的index，后者只是帧Index
		uint32_t swapchainIndex = 0;

		if (bResizeBuffer)
		{
			ResizeBuffer();
			return;
		}

		_vulkanManager->WaitForFences({ _executeFence[_currentFrameIndex] });

		if (!_vulkanManager->GetNextSwapchainIndex(_swapchain, _acquireSemaphore[_currentFrameIndex], nullptr, &swapchainIndex))
		{
			ResizeBuffer();
			return;
		}

		//Update Renderer
		{
			auto& cmdBuf = _cmdBuf[_currentFrameIndex];
			_vulkanManager->BeginCommandBuffer(cmdBuf);

			VkSemaphore wait = _acquireSemaphore[_currentFrameIndex];
			for (auto& i : _renderers)
			{
				auto nextWait = i.second->Render(wait);
				//if (nextWait != nullptr)
				//{
				//	wait = nextWait;
				//}
			}

			//把MainCamera绘制完的图像复制到swapchain
			{
				auto& world = _renderers.begin()->second->_world;

				CameraComponent* mainCamera = world->GetMainCamera();

				#if IS_EDITOR
				if (mainCamera == nullptr) //编辑器内，主相机无效的情况下,强制切换到编辑器相机
					mainCamera = world->_editorCamera;
				#endif
				auto& mainPassManager = _renderers.begin()->second->_passManagers[mainCamera];

				//Editor GUI Pass
				#if IS_EDITOR
				_imguiPassEditor->PassUpdate(mainPassManager->GetSceneTexture()->GetTexture(SceneTextureDesc::FinalColor));
				#else
				//编辑器使用Imgui制作，ImguiPassEditor会传递最后结果到Swapchain，不需要复制。
				mainPassManager->CmdCopyFinalColorToSwapchain();
				#endif

				_vulkanManager->EndCommandBuffer(cmdBuf);
				_vulkanManager->SubmitQueueForPasses(cmdBuf, wait, _queueSemaphore[_currentFrameIndex], _executeFence[_currentFrameIndex]);
				//Imgui如果开启了多视口模式，当控件离开窗口之后，会自动生成一套绘制流程，放在此处执行最后的处理会安全很多。
				_imguiPassEditor->EndFrame();
			}
		}

		//Present swapchain.
		if (!_vulkanManager->Present(_swapchain, _queueSemaphore[_currentFrameIndex], swapchainIndex))
		{
			ResizeBuffer();
			return;
		}

		//Get next frame index.
		_currentFrameIndex = (_currentFrameIndex + 1) % _vulkanManager->GetSwapchainBufferCount();
	}
}

bool VulkanSwapchain::ResizeBuffer()
{
	_vulkanManager->DeviceWaitIdle();

	_vulkanManager->DestroySwapchain(_swapchain, _swapchainImageViews);
#if __ANDROID__
	_Sleep(200);
#endif
	//部分情况下重置窗口会出现Surface被销毁的问题，最好Surface也重新创建一个
	_vulkanManager->ReCreateSurface_SDL(_windowHandle, _surface);

	//获取surfaceCapabilities，顺便检查一下Surface是否已经丢失了
	if (!_vulkanManager->GetSurfaceCapabilities(_surface, &_surfaceCapabilities))
	{
		return false;
	}

	_vulkanManager->GetSurfaceSize(_surface, _cacheSurfaceSize);

	if (_cacheSurfaceSize.width <= 0 || _cacheSurfaceSize.height <= 0)
	{
		bResizeBuffer = true;
		return false;
	}

	_cacheSurfaceSize = _surfaceSize = _vulkanManager->CreateSwapchain(
		_windowHandle,
		{ 1,1 },
		_surface,
		_surfaceFormat,
		_swapchain,
		_swapchainImages,
		_swapchainImageViews,
		_surfaceCapabilities,
		false,
		true);

	ResetResource();

	_imguiPassEditor->CheckWindowValid();
	for (auto& i : _renderers)
	{
		i.second->ResetRenderer();
	}
	_imguiPassEditor->PassReset();

	bResizeBuffer = false;

	//重置buffer会导致画面丢失，我们要在这一瞬间重新把buffer绘制回去，缓解缩放卡顿。
	_currentFrameIndex = 0;
	Update();

	return true;
}

void VulkanSwapchain::ResetResource()
{
	if (_acquireSemaphore.size() != _vulkanManager->GetSwapchainBufferCount())
	{
		//ConsoleDebug::print_endl("hBBr:Swapchain: Present Semaphore.");
		for (int i = 0; i < (int)_acquireSemaphore.size(); i++)
		{
			_vulkanManager->DestroySemaphore(_acquireSemaphore.at(i));
			_acquireSemaphore.at(i) = nullptr;
		}
		_acquireSemaphore.resize(_vulkanManager->GetSwapchainBufferCount());
		for (int i = 0; i < (int)_vulkanManager->GetSwapchainBufferCount(); i++)
		{
			_vulkanManager->CreateVkSemaphore(_acquireSemaphore.at(i));
		}
	}
	if (_queueSemaphore.size() != _vulkanManager->GetSwapchainBufferCount())
	{
		//ConsoleDebug::print_endl("hBBr:Swapchain: Present Semaphore.");
		for (int i = 0; i < (int)_queueSemaphore.size(); i++)
		{
			_vulkanManager->DestroySemaphore(_queueSemaphore.at(i));
			_queueSemaphore.at(i) = nullptr;
		}
		_queueSemaphore.resize(_vulkanManager->GetSwapchainBufferCount());
		for (int i = 0; i < (int)_vulkanManager->GetSwapchainBufferCount(); i++)
		{
			_vulkanManager->CreateVkSemaphore(_queueSemaphore.at(i));
		}
	}
	if (_cmdBuf.size() != _vulkanManager->GetSwapchainBufferCount())
	{
		//ConsoleDebug::print_endl("hBBr:Swapchain: Allocate Main CommandBuffers.");
		for (int i = 0; i < (int)_cmdBuf.size(); i++)
		{
			_vulkanManager->FreeCommandBuffer(_vulkanManager->GetCommandPool(), _cmdBuf.at(i));
			_cmdBuf.at(i) = nullptr;
		}
		_cmdBuf.resize(_vulkanManager->GetSwapchainBufferCount());
		for (int i = 0; i < (int)_vulkanManager->GetSwapchainBufferCount(); i++)
		{
			_vulkanManager->AllocateCommandBuffer(_vulkanManager->GetCommandPool(), _cmdBuf.at(i));
		}
	}
	if (_executeFence.size() != _vulkanManager->GetSwapchainBufferCount())
	{
		//ConsoleDebug::print_endl("hBBr:Swapchain: image acquired fences.");
		for (int i = 0; i < (int)_executeFence.size(); i++)
		{
			_vulkanManager->DestroyFence(_executeFence.at(i));
			_executeFence.at(i) = nullptr;
		}
		_executeFence.resize(_vulkanManager->GetSwapchainBufferCount());
		for (int i = 0; i < (int)_vulkanManager->GetSwapchainBufferCount(); i++)
		{
			_vulkanManager->CreateFence(_executeFence.at(i));
		}
	}
}

