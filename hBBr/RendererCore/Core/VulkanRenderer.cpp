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

#if IS_EDITOR
#include "ShaderCompiler.h"
#endif

std::map<HString, VulkanRenderer*>		VulkanRenderer::_renderers;
std::map<void*, std::function<void(VulkanRenderer* renderer, KeyCode key, KeyMod mod, Action action)>> VulkanRenderer::_key_inputs;
std::map<void*, std::function<void(VulkanRenderer* renderer, MouseButton mouse, Action action)>> VulkanRenderer::_mouse_inputs;

VulkanRenderer::VulkanRenderer(SDL_Window* windowHandle, const char* rendererName)
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
	_spwanNewWorld.clear();
	VulkanManager* _vulkanManager = VulkanManager::GetManager();
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	_world.reset();
	for (auto& i : _passManagers)
	{
		i.second.reset();
	}
	_passManagers.clear();
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), _cmdBuf);
	_vulkanManager->DestroySwapchain(_swapchain, _swapchainImageViews);
	_vulkanManager->DestroyRenderFences(_executeFence);
	_vulkanManager->DestroyRenderSemaphores(_presentSemaphore);
	_vulkanManager->DestroyRenderSemaphores(_queueSubmitSemaphore);
	//_vulkanManager->DestroyRenderFences(_imageAcquiredFences);
	_vulkanManager->DestroySurface(_surface);
	VulkanRenderer::_renderers.erase(GetName());
	ConsoleDebug::print_endl(HString("Release Renderer : ") + _rendererName);
	delete this;
}

void VulkanRenderer::Init()
{	
	#if IS_GAME
		_bIsInGame = true;
	#endif

	VulkanManager* _vulkanManager = VulkanManager::GetManager();

	//Surface
	ConsoleDebug::print_endl("hBBr:Start init Surface.");
	_vulkanManager->ReCreateSurface_SDL(_windowHandle, _surface);
	_vulkanManager->GetSurfaceCapabilities(_surface, &_surfaceCapabilities);

	//Swapchain
	ConsoleDebug::print_endl("hBBr:Start Check Surface Format.");
	_vulkanManager->CheckSurfaceFormat(_surface, _surfaceFormat);
	ConsoleDebug::print_endl("hBBr:Start Create Swapchain.");
	_renderSize =  _surfaceSize = _vulkanManager->CreateSwapchain(_windowSize, _surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews, _surfaceCapabilities, &_cmdBuf, &_presentSemaphore ,&_queueSubmitSemaphore,&_executeFence);

	//Set renderer map , Add new renderer
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	if (_renderers.size() <= 0)
	{
		_bIsMainRenderer = true;
	}
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
			MessageOut((HString("Has the same name of renderer.Random a new name is [") + _rendererName + "]"), false, false);
		}
	}
	ConsoleDebug::print_endl(HString("Create Renderer : ") + _rendererName);
	_renderThreadFuncsOnce.reserve(10);
	_renderThreadFuncs.reserve(10);

}

void VulkanRenderer::ReleaseWorld()
{
	if (_world)
	{
		_world.reset();
	}
}

bool VulkanRenderer::LoadWorld(HString worldNameOrGUID)
{
	if (_world)
	{
		_world.reset();
	}
	//检查GUID
	HString worldPath = FileSystem::GetWorldAbsPath();
	auto worldFolders = FileSystem::GetAllFolders(worldPath.c_str());
	FileEntry worldFolder;
	if (HGUID(worldNameOrGUID.c_str()).isValid())
	{
		for (auto& i : worldFolders)
		{
			HGUID input = HGUID(worldNameOrGUID.c_str());
			HGUID guid = HGUID(i.baseName.c_str());
			if (guid == input)
			{
				worldFolder = i;
				break;
			}

		}
	}
	else//不是GUID，那就看看是不是World名字
	{
		for (auto& i : worldFolders)
		{
			HString worldSettingPath = FileSystem::Append(i.absPath, ".WorldSettings");
			nlohmann::json j;
			if (Serializable::LoadJson(worldSettingPath, j))
			{
				auto it = j.find("WorldName");
				if (it != j.end())
				{
					HString name = it.value();
					if (name == worldNameOrGUID)
					{
						worldFolder = i;
						break;
					}
				}			
			}
		}

	}
	if(worldFolder.type != FileEntryType::Unknow)
	{
		_world.reset(new World());
		_world->_guidStr = worldFolder.baseName;
		StringToGUID(_world->_guidStr.c_str(), &_world->_guid);
		_world->_worldAbsPath = FileSystem::Append(worldPath, _world->_guidStr  + ".world");
		_world->_worldSettingAbsPath = FileSystem::Append(worldFolder.absPath, ".WorldSettings");
		_world->ReloadWorldSetting();
		for (auto f : _spwanNewWorld)
		{
			f(_world);
		}
		_world->Load(this);
		return true;
	}
	return false;
}

void VulkanRenderer::CreateEmptyWorld()
{
	_world = World::CreateNewWorld("NewWorld");
	_world->Load(this);
}

void VulkanRenderer::Render()
{
	VulkanManager* manager = VulkanManager::GetManager();
	if (!_bInit) //Render loop Init.
	{
		if (_bIsMainRenderer)
		{
			auto defaultWorldGUID = GetRendererConfig("Default", "DefaultWorld");
			LoadWorld(defaultWorldGUID);
		}

		if (!_world)
		{
			CreateEmptyWorld();
		}

		_bInit = true;
	}
	else if (!_bRendererRelease && _bInit)
	{
		if (bResizeBuffer)
		{
			ResizeBuffer();
			return;
		}
		if(_swapchain)
		{
			//这个 _swapchainIndex 和 _currentFrameIndex 不是一个东西，前者是有效交换链的index，后者只是帧Index
			uint32_t swapchainIndex = 0;

			if (!manager->GetNextSwapchainIndex(_swapchain, _presentSemaphore[_currentFrameIndex], VK_NULL_HANDLE, &swapchainIndex))
			{
				ResizeBuffer();
				return;
			}

			auto funcOnce = std::move(_renderThreadFuncsOnce);
			for (auto& func : funcOnce)
			{
				func();
			}
			for (auto& func : _renderThreadFuncs)
			{
				func();
			}

			if (_world)
				_world->WorldUpdate();


			manager->WaitForFences({ _executeFence[_currentFrameIndex] });
			auto cmdBuf = _cmdBuf[_currentFrameIndex];
			manager->ResetCommandBuffer(cmdBuf);
			manager->BeginCommandBuffer(cmdBuf);

			for (auto& p : _passManagers)
			{
				p.second->SetupPassUniformBuffer(p.first, _renderSize);
				p.second->PassesUpdate();
			}

			//把MainCamera绘制完的图像复制到swapchain
			if (_world)
			{
				VulkanManager* manager = VulkanManager::GetManager();
				CameraComponent* mainCamera = _world->_mainCamera;
				#if IS_EDITOR
				if (mainCamera == nullptr) //编辑器内，主相机无效的情况下,强制切换到编辑器相机
					mainCamera = _world->_editorCamera;
				#endif
				auto& mainPassManager = _passManagers[mainCamera];
				mainPassManager->CmdCopyFinalColorToSwapchain();
			}

			manager->EndCommandBuffer(cmdBuf);
			manager->SubmitQueueForPasses(cmdBuf, GetPresentSemaphore(), GetSubmitSemaphore(), _executeFence[_currentFrameIndex]);

			//Present swapchain.
			if (!manager->Present(_swapchain, _queueSubmitSemaphore[_currentFrameIndex], swapchainIndex))
			{
				ResizeBuffer();
				return;
			}

			//Get valid frame index
			_lastValidSwapchainIndex = _currentFrameIndex;

			//Get next frame index.
			uint32_t maxNumSwapchainImages = manager->GetSwapchainBufferCount();
			_currentFrameIndex = (_currentFrameIndex + 1) % maxNumSwapchainImages;
		}
	}
}

void VulkanRenderer::RendererResize(uint32_t w, uint32_t h)
{
	_windowSize.width = w;
	_windowSize.height = h;
	bResizeBuffer = true;
	ResizeBuffer();
}

bool VulkanRenderer::ResizeBuffer()
{
	_lastValidSwapchainIndex = -1;
	if (_windowSize.width<=0 && _windowSize.height <= 0)
	{
		_windowSize = VulkanManager::GetManager()->GetSurfaceSize(_windowSize, _surface);
	}
	if (!_bRendererRelease && _windowSize.width > 0 && _windowSize.height > 0)
	{
		VulkanManager* _vulkanManager = VulkanManager::GetManager();
		{
			vkDeviceWaitIdle(_vulkanManager->GetDevice());
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

			_vulkanManager->CheckSurfaceFormat(_surface, _surfaceFormat);
			_renderSize = _surfaceSize = _vulkanManager->CreateSwapchain(_windowSize, _surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews, _surfaceCapabilities, &_cmdBuf, &_presentSemaphore, &_queueSubmitSemaphore
			, &_executeFence, false, true);
			if (_swapchain == VK_NULL_HANDLE)
			{
				return false;
			}

			for (auto& p : _passManagers)
			{
				p.second->PassesReset();
			}
			bResizeBuffer = false;

			//重置buffer会导致画面丢失，我们要在这一瞬间重新把buffer绘制回去，缓解缩放卡顿。
			//for (int i = 0; i < (int)_vulkanManager->GetSwapchainBufferCount(); i++)
			//{
				Render();
			//}

			return true;
		}
	}
	return false;
}
