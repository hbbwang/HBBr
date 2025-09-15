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
	ConsoleDebug::print_endl(std::string("Release Renderer : ") + _rendererName);
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
}

void VulkanRenderer::ReleaseWorld()
{
	if (_world)
	{
		_world.reset();
	}
}

bool VulkanRenderer::LoadWorld(std::string worldNameOrGUID)
{
	if (_world)
	{
		_world.reset();
	}
	//检查GUID
	std::string worldPath = FileSystem::GetWorldAbsPath();
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
			std::string worldSettingPath = FileSystem::Append(i.absPath, ".WorldSettings");
			nlohmann::json j;
			if (Serializable::LoadJson(worldSettingPath, j))
			{
				auto it = j.find("WorldName");
				if (it != j.end())
				{
					std::string name = it.value();
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

void VulkanRenderer::Update()
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
		
	}
	else if (!_bRendererRelease && _bInit)
	{
		if (_world)
			_world->WorldUpdate();
	}
	_cpuTime = _cpuTimer.End_ms();
}

VkSemaphore VulkanRenderer::Render(VkSemaphore wait)
{	
	if (!_bRendererRelease && _bInit)
	{
		uint32_t frameIndex = _swapchain->GetCurrentFrameIndex();

		if(_swapchain)
		{
			for (auto& func : _renderThreadFuncsOnce)
			{
				func();
			}
			_renderThreadFuncsOnce.clear();

			for (auto& func : _renderThreadFuncs)
			{
				func();
			}

			for (auto& p : _passManagers)
			{
				p.second->SetupPassUniformBuffer(p.first, _renderSize);
				p.second->PassesUpdate();
			}
		}
	}

	return nullptr;
}
