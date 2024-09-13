#pragma once
#include "VulkanManager.h"
#include "FormMain.h"
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <functional>
#include "Pass/PassType.h"
#include "Asset/HGuid.h"
#include "HTime.h"
#include "HInput.h"
#include "HBox.h"
#include "VulkanSwapchain.h"

class Texture2D;
class Scene;

class VulkanRenderer
{
	friend class PassManager;
	friend class VulkanApp;
	friend class CameraComponent;
	friend class VulkanSwapchain;
public:
	HBBR_API VulkanRenderer(VulkanSwapchain* swapchain, const char* rendererName);

	HBBR_API ~VulkanRenderer();

	HBBR_INLINE bool IsRendererWantRelease() {
		return _bRendererRelease;
	}

	HBBR_API HBBR_INLINE std::map<class CameraComponent*, std::shared_ptr<class PassManager>> GetPassManagers() {
		return _passManagers;
	}

	//渲染分辨率
	HBBR_API HBBR_INLINE VkExtent2D GetRenderSize()const {
		return _renderSize;
	}

	HBBR_API HBBR_INLINE void SetRenderSize(VkExtent2D newSize) {
		_renderSize = newSize;
	}

	HBBR_API HBBR_INLINE bool IsInit() {
		return _bInit;
	}

	HBBR_API HBBR_INLINE HString GetName() {
		return _rendererName;
	}

	HBBR_INLINE VkCommandBuffer& GetCommandBuffer(){
		return _swapchain->_cmdBuf[_swapchain->_currentFrameIndex];
	}

	HBBR_INLINE const HBox2D GetRendererRegion() const{
		return _rendererRegion;
	}

	HBBR_API HBBR_INLINE bool IsWorldValid()const{
		if (_world)
			return true;
		else
			return false;
	}

	HBBR_API HBBR_INLINE class std::weak_ptr<class World> GetWorld() {
		if (IsWorldValid()){
			return _world;
		}
		else{
			return std::weak_ptr<class World>();
		}
	}

	HBBR_API HBBR_INLINE void ExecFunctionOnRenderThread(std::function<void()> func)
	{
		if (_renderThreadFuncsOnce.capacity() <= _renderThreadFuncsOnce.size())
		{
			_renderThreadFuncsOnce.reserve(_renderThreadFuncsOnce.capacity() + 4);
		}
		_renderThreadFuncsOnce.push_back(func);
	}

	HBBR_API HBBR_INLINE bool IsInGame()const
	{
		return _bIsInGame;
	}

	HBBR_API void ReleaseWorld();

	HBBR_API bool LoadWorld(HString worldNameOrGUID);

	HBBR_API void CreateEmptyWorld();

	HBBR_INLINE void SetupKeyInput(void* ptr , std::function<void(class VulkanRenderer* renderer, KeyCode key, KeyMod mod, Action action)>func)
	{
		_key_inputs.emplace(ptr, func);
	}

	HBBR_INLINE void SetupMouseInput(void* ptr, std::function<void(class VulkanRenderer* renderer, MouseButton mouse, Action action)>func)
	{
		_mouse_inputs.emplace(ptr, func);
	}

	HBBR_INLINE void RemoveKeyInput(void* ptr)
	{
		_key_inputs.erase(ptr);
	}

	HBBR_INLINE void RemoveMouseInput(void* ptr)
	{
		_mouse_inputs.erase(ptr);
	}

	HBBR_API HBBR_INLINE bool IsMainRenderer()
	{
		return _bIsMainRenderer;
	}

	HBBR_API HBBR_INLINE bool HasInputFocus()
	{
		return bHasInputFocus;
	}

	HBBR_API HBBR_INLINE VulkanSwapchain* GetSwapchain()
	{
		return _swapchain;
	}

	/* 帧渲染函数 */
	HBBR_API VkSemaphore Render(VkSemaphore wait);

	HBBR_API void Release();

	HBBR_INLINE double GetCPURenderingTime()const { return _cpuTime; }

	void Init();

	std::vector<std::function<void(std::weak_ptr<World>)>> _spwanNewWorld;

private:

	void ResetRenderer();

	void ResetResource();

	HString _rendererName;

	VkExtent2D _renderSize{};

	// std::vector<VkFence> _executeFence;

	// std::vector<VkSemaphore> _queueSubmitSemaphore;

	// std::vector<VkCommandBuffer> _cmdBuf;

	bool _bRendererRelease;

	bool _bInit;

	//Passes，多少个相机，就有多少组passes需要渲染
	std::map<class CameraComponent*, std::shared_ptr<class PassManager>> _passManagers;

	//World
	std::shared_ptr<class World> _world;

	std::vector<std::function<void()>> _renderThreadFuncsOnce;

	std::vector<std::function<void()>> _renderThreadFuncs;

	bool _bIsInGame;

	//主渲染器，默认第一个创建的是，它会自动加载配置场景
	bool _bIsMainRenderer;

	//Input
	bool bHasInputFocus;
	std::map <void*, std::function<void(class VulkanRenderer* renderer, KeyCode key, KeyMod mod, Action action)>> _key_inputs;
	std::map <void*, std::function<void(class VulkanRenderer* renderer, MouseButton mouse, Action action)>> _mouse_inputs;

	VulkanSwapchain* _swapchain;

	VulkanManager* _vulkanManager;

	HBox2D _rendererRegion;

	//性能测试
	HTime _cpuTimer;
	double _cpuTime;

};