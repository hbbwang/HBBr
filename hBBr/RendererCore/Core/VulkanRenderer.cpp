#include "VulkanRenderer.h"
#include "Texture2D.h"
#include "RendererConfig.h"
#include "ConsoleDebug.h"
#include "PassManager.h"
#include "PassBase.h"
#include "FileSystem.h"
#include "Shader.h"
#include "Thread.h"
#include "Asset/World.h"
#include "Component/GameObject.h"
#include "Component/CameraComponent.h"

#if IS_EDITOR
#include "ShaderCompiler.h"
#endif

std::map<HString, VulkanRenderer*>		VulkanRenderer::_renderers;
uint32_t								VulkanRenderer::_currentFrameIndex;

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
	_passManager.reset();
	_world.reset();
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), _cmdBuf);
	_vulkanManager->DestroySwapchain(_swapchain, _swapchainImageViews);
	_vulkanManager->DestroyRenderSemaphores(_presentSemaphore);
	_vulkanManager->DestroyRenderSemaphores(_queueSubmitSemaphore);
	//_vulkanManager->DestroyRenderFences(_imageAcquiredFences);
	_vulkanManager->DestroySurface(_surface);
	VulkanRenderer::_renderers.erase(GetName());
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

	//Swapchain
	ConsoleDebug::print_endl("hBBr:Start Check Surface Format.");
	_vulkanManager->CheckSurfaceFormat(_surface, _surfaceFormat);
	ConsoleDebug::print_endl("hBBr:Start Create Swapchain.");
	_surfaceSize = _vulkanManager->CreateSwapchain(_windowSize, _surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews, _surfaceCapabilities, &_cmdBuf, &_presentSemaphore ,&_queueSubmitSemaphore);

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

	_renderThreadFuncsOnce.reserve(10);
	_renderThreadFuncs.reserve(10);
	 
	//Init passes
	_passManager.reset(new PassManager());

	//Start GameTime
	_gameTime.Start();

}

void VulkanRenderer::LoadWorld(HString worldNameOrGUID)
{
	auto& worlds = World::GetWorlds();
	//检查GUID
	if (HGUID(worldNameOrGUID.c_str()).isValid())
	{
		for (auto& i : worlds)
		{
			auto worldGuidStr = HString(i.second->_guid.str().c_str());
			if (worldGuidStr == worldNameOrGUID)
			{
				_world = i.second;
				for (auto f : _spwanNewWorld)
				{
					f(_world);
				}
				_world.lock()->Load(this);
				break;
			}
		}
	}
	else
	{
		auto it = worlds.find(worldNameOrGUID);
		if (it != worlds.end())
		{
			_world = it->second;
			for (auto i : _spwanNewWorld)
			{
				i(_world);
			}
			_world.lock()->Load(this);
		}
	}

}

void VulkanRenderer::CreateEmptyWorld()
{
	_world = World::CreateNewWorld("NewWorld");
	_world.lock()->Load(this);
}

void VulkanRenderer::Render()
{
	if (!_bInit) //Render loop Init.
	{
		auto defaultWorldGUID = GetRendererConfig("Default", "DefaultWorld");
		LoadWorld(defaultWorldGUID);

		if (_world.expired())
		{
			CreateEmptyWorld();
		}
		_passManager->PassesInit(this);
		_bInit = true;
	}
	else if (!_bRendererRelease && _bInit)
	{
		if (bResizeBuffer)
		{
			ResizeBuffer();
		}

		_frameRate = _frameTime.FrameRate_ms();

		VulkanManager* _vulkanManager = VulkanManager::GetManager();

		uint32_t _swapchainIndex = 0;

		if (!_vulkanManager->GetNextSwapchainIndex(_swapchain, _presentSemaphore[_currentFrameIndex], VK_NULL_HANDLE, &_swapchainIndex))
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

		if (!_world.expired())
			_world.lock()->WorldUpdate();

		SetupPassUniformBuffer();

		_passManager->PassesUpdate();

		//Present swapchain.
		if (!_vulkanManager->Present(_swapchain, _queueSubmitSemaphore[_currentFrameIndex], _swapchainIndex))
		{
			ResizeBuffer();
			return;
		}

		//Get next frame index.
		uint32_t maxNumSwapchainImages = _vulkanManager->GetSwapchainBufferCount();
		_currentFrameIndex = (_currentFrameIndex + 1) % maxNumSwapchainImages;
	}
}

void VulkanRenderer::RendererResize(uint32_t w, uint32_t h)
{
	_windowSize.width = w;
	_windowSize.height = h;
	bResizeBuffer = true;
	ResizeBuffer();
}

void VulkanRenderer::SetupPassUniformBuffer()
{
	const CameraComponent* mainCamera = nullptr;
	if (!IsWorldValid())
	{
		return;
	}
	if (_bIsInGame)
		mainCamera = GetWorld()->_mainCamera;
#if IS_EDITOR
	else
		mainCamera = GetWorld()->_editorCamera;
#endif
	if (mainCamera != nullptr)
	{
		_passUniformBuffer.View = mainCamera->GetViewMatrix();
		_passUniformBuffer.View_Inv = mainCamera->GetInvViewMatrix();
		float aspect = (float)_surfaceSize.width / (float)_surfaceSize.height;
		//DirectX Left hand 
		glm::mat4 flipYMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 0.5f));

		{//Screen Rotator
			glm::mat4 pre_rotate_mat = glm::mat4(1);
			glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
			if (_surfaceCapabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
				pre_rotate_mat = glm::rotate(pre_rotate_mat, glm::radians(90.0f), rotation_axis);
                aspect = (float)_surfaceSize.height / (float)_surfaceSize.width;
			}
			else if (_surfaceCapabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
				pre_rotate_mat = glm::rotate(pre_rotate_mat, glm::radians(270.0f), rotation_axis);
                aspect = (float)_surfaceSize.height / (float)_surfaceSize.width;
			}
			else if (_surfaceCapabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
				pre_rotate_mat = glm::rotate(pre_rotate_mat, glm::radians(180.0f), rotation_axis);
			}
            //DirectX Left hand.
            _passUniformBuffer.Projection = glm::perspectiveLH(glm::radians(mainCamera->GetFOV()), aspect, mainCamera->GetNearClipPlane(), mainCamera->GetFarClipPlane());
			_passUniformBuffer.Projection = pre_rotate_mat * flipYMatrix * _passUniformBuffer.Projection;
		}

		_passUniformBuffer.Projection_Inv = glm::inverse(_passUniformBuffer.Projection);
		_passUniformBuffer.ViewProj = _passUniformBuffer.Projection * _passUniformBuffer.View;

		_passUniformBuffer.ViewProj_Inv = glm::inverse(_passUniformBuffer.ViewProj);
		_passUniformBuffer.ScreenInfo = glm::vec4((float)_surfaceSize.width, (float)_surfaceSize.height, mainCamera->GetNearClipPlane(), mainCamera->GetFarClipPlane());
		auto trans = mainCamera->GetGameObject()->GetTransform();
		_passUniformBuffer.CameraPos_GameTime = glm::vec4(trans->GetWorldLocation().x, trans->GetWorldLocation().y, trans->GetWorldLocation().z, (float)GetGameTime());
		auto viewDir = glm::normalize(trans->GetForwardVector());
		_passUniformBuffer.CameraDirection = glm::vec4(viewDir.x, viewDir.y, viewDir.z, 0.0f);

	}
}

bool VulkanRenderer::ResizeBuffer()
{
	if (!_bRendererRelease)
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
			_vulkanManager->CheckSurfaceFormat(_surface, _surfaceFormat);
			_surfaceSize = _vulkanManager->CreateSwapchain(_windowSize, _surface, _surfaceFormat, _swapchain, _swapchainImages, _swapchainImageViews, _surfaceCapabilities, &_cmdBuf, &_presentSemaphore, &_queueSubmitSemaphore
			,nullptr,false,true);
			if (_swapchain == VK_NULL_HANDLE)
			{
				return false;
			}
			_passManager->PassesReset();
			bResizeBuffer = false;

			//重置buffer会导致画面丢失，我们要在这一瞬间重新把buffer绘制回去，缓解黑屏。
			_currentFrameIndex = 0;
			for (int i = 0; i < (int)_vulkanManager->GetSwapchainBufferCount(); i++)
			{
				Render();
			}

			return true;
		}
	}
	return false;
}
