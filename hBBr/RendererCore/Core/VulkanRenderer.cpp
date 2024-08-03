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

std::map<HString, VulkanRenderer*>		VulkanRenderer::_renderers;
std::map<void*, std::function<void(VulkanRenderer* renderer, KeyCode key, KeyMod mod, Action action)>> VulkanRenderer::_key_inputs;
std::map<void*, std::function<void(VulkanRenderer* renderer, MouseButton mouse, Action action)>> VulkanRenderer::_mouse_inputs;

VulkanRenderer::VulkanRenderer(SDL_Window* windowHandle, const char* rendererName):
	_vulkanManager(VulkanManager::GetManager()),
	_currentFrameIndex(0),
	_bRendererRelease(false),
	_windowHandle(windowHandle),
	_rendererName(rendererName),
	_swapchain(nullptr),
	_surface(nullptr),
	_bInit(false)
{
	Init();
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::Release()
{
	_bRendererRelease = true;
	_spwanNewWorld.clear();
	vkDeviceWaitIdle(_vulkanManager->GetDevice());
	_world.reset();
	for (auto& i : _passManagers)
	{
		i.second.reset();
	}
	_passManagers.clear();
	//Remove material
	for (auto& p : PrimitiveProxy::GetAllMaterialPrimitiveArray())
	{
		for (auto& m : p)
		{
			auto it = m->_materialPrimitiveGroups.find(this);
			if (it != m->_materialPrimitiveGroups.end())
			{
				delete it->second;
				it->second = nullptr;
			}
			m->_materialPrimitiveGroups.erase(this);
		}
	}
	//
	_imguiPass.reset();
#if IS_EDITOR
	_imguiPassEditor.reset();
#endif
	_vulkanManager->FreeCommandBuffers(_vulkanManager->GetCommandPool(), _cmdBuf);
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

	//Surface
	ConsoleDebug::print_endl("hBBr:Start init Surface.");
	_vulkanManager->ReCreateSurface_SDL(_windowHandle, _surface);
	_vulkanManager->GetSurfaceCapabilities(_surface, &_surfaceCapabilities);

	//Swapchain
	ConsoleDebug::print_endl("hBBr:Start Check Surface Format.");
	_vulkanManager->CheckSurfaceFormat(_surface, _surfaceFormat);
	ConsoleDebug::print_endl("hBBr:Start Create Swapchain.");
	_renderSize = _surfaceSize = _vulkanManager->CreateSwapchain(
		_windowHandle,
		{ 1,1 },
		_surface,
		_surfaceFormat,
		_swapchain,
		_swapchainImages,
		_swapchainImageViews,
		_surfaceCapabilities,
		&_cmdBuf,
		&_presentSemaphore,
		&_queueSubmitSemaphore,
		& _executeFence);

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
	if (_world)
	{
		_world.reset();
	}
	_world = World::CreateNewWorld("NewWorld");
	_world->Load(this);
}
void VulkanRenderer::Render()
{
	_cpuTimer.Start();
	if (!_bInit) //Render loop Init.
	{
		_bInit = true;

		if (_bIsMainRenderer)
		{
			auto defaultWorldGUID = GetRendererConfig("Default", "DefaultWorld");
			LoadWorld(defaultWorldGUID);
		}

		if (!_world)
		{
			CreateEmptyWorld();
		}

		_imguiPass.reset(new ImguiPass(this));
		_imguiPass->SetPassName("GUI Pass");
		_imguiPass->PassInit();

		#if IS_EDITOR
		_imguiPassEditor.reset(new ImguiPassEditor(this));
		_imguiPassEditor->SetPassName("Editor GUI Pass");
		_imguiPassEditor->PassInit();
		#endif
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

			for (auto& func : _renderThreadFuncsOnce)
			{
				func();
			}
			_renderThreadFuncsOnce.clear();

			for (auto& func : _renderThreadFuncs)
			{
				func();
			}

			if (_world)
				_world->WorldUpdate();

			_vulkanManager->WaitForFences({ _executeFence[_currentFrameIndex] });

			if (!_vulkanManager->GetNextSwapchainIndex(_swapchain, _presentSemaphore[_currentFrameIndex], nullptr, &swapchainIndex))
			{
				ResizeBuffer();
				return;
			}

			auto& cmdBuf = _cmdBuf[_currentFrameIndex];
			//_vulkanManager->ResetCommandBuffer(cmdBuf);
			_vulkanManager->BeginCommandBuffer(cmdBuf);

			for (auto& p : _passManagers)
			{
				p.second->SetupPassUniformBuffer(p.first, _renderSize);
				p.second->PassesUpdate();
			}

			//把MainCamera绘制完的图像复制到swapchain
			if (_world)
			{
				CameraComponent* mainCamera = _world->_mainCamera;

				#if IS_EDITOR
				if (mainCamera == nullptr) //编辑器内，主相机无效的情况下,强制切换到编辑器相机
					mainCamera = _world->_editorCamera;
				#endif
				auto& mainPassManager = _passManagers[mainCamera];

				//GUI Pass
				_imguiPass->PassUpdate({
					mainPassManager->GetSceneTexture()->GetTexture(SceneTextureDesc::FinalColor)
				});

				//Editor GUI Pass
				#if IS_EDITOR
				_imguiPassEditor->PassUpdate(mainPassManager->GetSceneTexture()->GetTexture(SceneTextureDesc::FinalColor));
				#else
				//编辑器使用Imgui制作，ImguiPassEditor会传递最后结果到Swapchain，不需要复制。
				mainPassManager->CmdCopyFinalColorToSwapchain();
				#endif		
			}

			_vulkanManager->EndCommandBuffer(cmdBuf);
			_vulkanManager->SubmitQueueForPasses(cmdBuf, _presentSemaphore[_currentFrameIndex], _queueSubmitSemaphore[_currentFrameIndex], _executeFence[_currentFrameIndex]);
			//Imgui如果开启了多视口模式，当控件离开窗口之后，会自动生成一套绘制流程，放在此处执行最后的处理会安全很多。
			_imguiPass->EndFrame();
			_imguiPassEditor->EndFrame();
			//Present swapchain.
			if (!_vulkanManager->Present(_swapchain, _queueSubmitSemaphore[_currentFrameIndex], swapchainIndex))
			{
				ResizeBuffer();
				return;
			}

			//Get next frame index.
			_currentFrameIndex = (_currentFrameIndex + 1) % _vulkanManager->GetSwapchainBufferCount();
		}
	}
	_cpuTime = _cpuTimer.End_ms();
}

void VulkanRenderer::RendererResize(uint32_t w, uint32_t h)
{
	bResizeBuffer = true;
}

bool VulkanRenderer::ResizeBuffer()
{
	if (!_bRendererRelease)
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

		_renderSize = _cacheSurfaceSize = _surfaceSize = _vulkanManager->CreateSwapchain(
			_windowHandle, 
			{ 1,1 }, 
			_surface, 
			_surfaceFormat, 
			_swapchain, 
			_swapchainImages, 
			_swapchainImageViews, 
			_surfaceCapabilities, 
			&_cmdBuf, 
			&_presentSemaphore, 
			&_queueSubmitSemaphore, 
			&_executeFence, 
			false, 
			true);

		for (auto& p : _passManagers)
		{
			p.second->PassesReset();
		}
		_imguiPass->PassReset();
		_imguiPassEditor->PassReset();

		bResizeBuffer = false;

		//重置buffer会导致画面丢失，我们要在这一瞬间重新把buffer绘制回去，缓解缩放卡顿。
		_currentFrameIndex = 0;
		Render();

		return true;
	}
	return false;
}
