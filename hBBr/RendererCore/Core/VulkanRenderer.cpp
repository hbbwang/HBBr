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

VulkanRenderer::VulkanRenderer(VulkanSwapchain* swapchain, const char* rendererName):
	_vulkanManager(VulkanManager::GetManager()),
	_bRendererRelease(false),
	_rendererName(rendererName),
	_bInit(false),
	_swapchain(swapchain)
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

	//_vulkanManager->FreeCommandBuffers(_vulkanManager->GetCommandPool(), _cmdBuf);
	//_vulkanManager->DestroyRenderFences(_executeFence);
	//_vulkanManager->DestroyRenderSemaphores(_queueSubmitSemaphore);
	ConsoleDebug::print_endl(HString("Release Renderer : ") + _rendererName);
	delete this;
}

void VulkanRenderer::Init()
{	
	#if IS_GAME
		_bIsInGame = true;
	#endif

	_renderThreadFuncsOnce.reserve(10);
	_renderThreadFuncs.reserve(10);

	ResetResource();
}

void VulkanRenderer::ResetRenderer()
{
	ResetResource();
	for (auto& p : _passManagers)
	{
		p.second->PassesReset();
	}
}

void VulkanRenderer::ResetResource()
{
	auto swapchainCount = _vulkanManager->GetSwapchainBufferCount();
	//init semaphores & fences
	//if (_queueSubmitSemaphore.size() != swapchainCount)
	//{
	//	//ConsoleDebug::print_endl("hBBr:Swapchain: Queue Submit Semaphore.");
	//	for (int i = 0; i < (int)_queueSubmitSemaphore.size(); i++)
	//	{
	//		_vulkanManager->DestroySemaphore(_queueSubmitSemaphore.at(i));
	//		_queueSubmitSemaphore.at(i) = nullptr;
	//	}
	//	_queueSubmitSemaphore.resize(swapchainCount);
	//	for (int i = 0; i < (int)swapchainCount; i++)
	//	{
	//		_vulkanManager->CreateVkSemaphore(_queueSubmitSemaphore.at(i));
	//	}
	//}
	//if (_executeFence.size() != swapchainCount)
	//{
	//	//ConsoleDebug::print_endl("hBBr:Swapchain: image acquired fences.");
	//	for (int i = 0; i < (int)_executeFence.size(); i++)
	//	{
	//		_vulkanManager->DestroyFence(_executeFence.at(i));
	//		_executeFence.at(i) = nullptr;
	//	}
	//	_executeFence.resize(swapchainCount);
	//	for (int i = 0; i < (int)swapchainCount; i++)
	//	{
	//		_vulkanManager->CreateFence(_executeFence.at(i));
	//	}
	//}
	//if (_cmdBuf.size() != _vulkanManager->GetSwapchainBufferCount())
	//{
	//	//ConsoleDebug::print_endl("hBBr:Swapchain: Allocate Main CommandBuffers.");
	//	for (int i = 0; i < (int)_cmdBuf.size(); i++)
	//	{
	//		_vulkanManager->FreeCommandBuffer(_vulkanManager->GetCommandPool(), _cmdBuf.at(i));
	//		_cmdBuf.at(i) = nullptr;
	//	}
	//	_cmdBuf.resize(_vulkanManager->GetSwapchainBufferCount());
	//	for (int i = 0; i < (int)_vulkanManager->GetSwapchainBufferCount(); i++)
	//	{
	//		_vulkanManager->AllocateCommandBuffer(_vulkanManager->GetCommandPool(), _cmdBuf.at(i));
	//	}
	//}
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

VkSemaphore VulkanRenderer::Render(VkSemaphore wait)
{
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
		return nullptr;
	}
	else if (!_bRendererRelease && _bInit)
	{
		_cpuTimer.Start();

		uint32_t frameIndex = _swapchain->GetCurrentFrameIndex();

		if(_swapchain)
		{
			//_vulkanManager->WaitForFences({ _executeFence[_currentFrameIndex] });

			//auto& cmdBuf = _cmdBuf[frameIndex];
			//_vulkanManager->BeginCommandBuffer(cmdBuf);

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

			for (auto& p : _passManagers)
			{
				p.second->SetupPassUniformBuffer(p.first, _renderSize);
				p.second->PassesUpdate();
			}

			//_vulkanManager->EndCommandBuffer(cmdBuf);
			//_vulkanManager->SubmitQueueForPasses(cmdBuf, wait, _queueSubmitSemaphore[frameIndex], _executeFence[frameIndex]);

		}
		_cpuTime = _cpuTimer.End_ms();
		// return _queueSubmitSemaphore[frameIndex];
	}

	return nullptr;
}
