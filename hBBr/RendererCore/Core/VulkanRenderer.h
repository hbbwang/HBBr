#pragma once
#include "VulkanManager.h"
#include "FormMain.h"
#include <memory>
#include <mutex>
#include <thread>
#include <functional>
#include <map>
#include "Pass/PassType.h"
#include "Asset/HGuid.h"
#include "HTime.h"
#include "HInput.h"

class Texture2D;
class Scene;

class VulkanRenderer
{
	friend class PassManager;
	friend class VulkanApp;
public:
	HBBR_API VulkanRenderer(SDL_Window* windowHandle, const char* rendererName);

	HBBR_API ~VulkanRenderer();

	/* Get current Frame buffer index. It is frequent use in the passes. */
	HBBR_API HBBR_INLINE int GetCurrentFrameIndex() {
		return _currentFrameIndex;
	}

	HBBR_INLINE bool IsRendererWantRelease() {
		return _bRendererRelease;
	}

	HBBR_API HBBR_INLINE class PassManager* GetPassManager() {
		return _passManager.get();
	}

	HBBR_API HBBR_INLINE SDL_Window* GetWindowHandle() {
		return _windowHandle;
	}

	HBBR_INLINE VkSurfaceFormatKHR GetSurfaceFormat()const {
		return _surfaceFormat;
	}

	/* Get swapchain imageViews for render attachments */
	HBBR_INLINE std::vector<VkImageView> GetSwapchainImageViews() {
		return _swapchainImageViews;
	}

	HBBR_API HBBR_INLINE VkExtent2D GetSurfaceSize()const {
		return _surfaceSize;
	}

	HBBR_INLINE VkSemaphore* GetPresentSemaphore() {
		return &_presentSemaphore[_currentFrameIndex];
	}

	HBBR_INLINE VkSemaphore* GetSubmitSemaphore() {
		return &_queueSubmitSemaphore[_currentFrameIndex];
	}

	HBBR_API HBBR_INLINE bool IsInit() {
		return _bInit;
	}

	HBBR_API HBBR_INLINE HString GetName() {
		return _rendererName;
	}

	HBBR_INLINE VkCommandBuffer GetCommandBuffer()const{
		return _cmdBuf[_currentFrameIndex];
	}

	HBBR_API HBBR_INLINE bool IsWorldValid()const{
		if (!_world.expired())
			return true;
		else
			return false;
	}

	HBBR_API HBBR_INLINE class World* GetWorld() {
		if (IsWorldValid()){
			return _world.lock().get();
		}
		else{
			return nullptr;
		}
	}

	HBBR_API HBBR_INLINE bool HasFocus() {
		return VulkanApp::IsWindowFocus(_windowHandle);
	}

	HBBR_INLINE const PassUniformBuffer& GetPassUniformBufferCache()
	{
		return _passUniformBuffer;
	}

	HBBR_API HBBR_INLINE void ExecFunctionOnRenderThread(std::function<void()> func)
	{
		_renderThreadFuncsOnce.push_back(func);
	}

	HBBR_API HBBR_INLINE bool IsInGame()const
	{
		return _bIsInGame;
	}

	HBBR_API void LoadWorld(HString worldNameOrGUID);

	HBBR_API void CreateEmptyWorld();

	HBBR_INLINE static void SetupKeyInput(void* ptr , std::function<void(class VulkanRenderer* renderer, KeyCode key, KeyMod mod, Action action)>func)
	{
		_key_inputs.emplace(ptr, func);
	}

	HBBR_INLINE static void SetupMouseInput(void* ptr, std::function<void(class VulkanRenderer* renderer, MouseButton mouse, Action action)>func)
	{
		_mouse_inputs.emplace(ptr, func);
	}

	HBBR_INLINE static void RemoveKeyInput(void* ptr)
	{
		_key_inputs.erase(ptr);
	}

	HBBR_INLINE static void RemoveMouseInput(void* ptr)
	{
		_mouse_inputs.erase(ptr);
	}

	HBBR_API HBBR_INLINE bool IsMainRenderer()
	{
		return _bIsMainRenderer;
	}

	/* 帧渲染函数 */
	HBBR_API void Render();

	HBBR_API void RendererResize(uint32_t w,uint32_t h);

	HBBR_API void Release();

	void Init();

	//Renderer map
	static std::map<HString, VulkanRenderer*> _renderers;

	VkSurfaceCapabilitiesKHR _surfaceCapabilities{};

	std::vector<std::function<void(std::weak_ptr<World>)>> _spwanNewWorld;

	bool bResizeBuffer;

private:

	void SetupPassUniformBuffer();

	bool ResizeBuffer();

	HString _rendererName;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkSurfaceFormatKHR _surfaceFormat{};

	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	VkExtent2D _surfaceSize{};

	VkExtent2D _windowSize{};

	std::vector<VkImage>	_swapchainImages;

	std::vector<VkImageView>_swapchainImageViews;

	std::vector<VkSemaphore> _presentSemaphore;

	std::vector<VkSemaphore> _queueSubmitSemaphore;

	//std::vector<VkFence> _imageAcquiredFences;

	uint32_t _currentFrameIndex;

	uint32_t _maxSwapchainImages = 0;

	bool _bRendererRelease;

	bool _bInit;

	std::vector<VkCommandBuffer> _cmdBuf;

	SDL_Window* _windowHandle = nullptr;

	//Pass Uniform
	PassUniformBuffer _passUniformBuffer;

	//Passes
	std::unique_ptr<class PassManager> _passManager;

	//World
	std::weak_ptr<class World> _world;

	std::vector<std::function<void()>> _renderThreadFuncsOnce;

	std::vector<std::function<void()>> _renderThreadFuncs;

	bool _bIsInGame;

	//主渲染器，默认第一个创建的是，它会自动加载配置场景
	bool _bIsMainRenderer;

	//Input
	static std::map < void*, std::function<void(class VulkanRenderer* renderer, KeyCode key, KeyMod mod, Action action)>> _key_inputs;
	static std::map < void*, std::function<void(class VulkanRenderer* renderer, MouseButton mouse, Action action)>> _mouse_inputs;

};