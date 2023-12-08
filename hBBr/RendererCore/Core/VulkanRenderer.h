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

class Texture;
class Scene;

class VulkanRenderer
{
	friend class PassManager;
public:
	HBBR_API VulkanRenderer(SDL_Window* windowHandle, const char* rendererName);

	HBBR_API ~VulkanRenderer();

	/* Get current Frame buffer index. It is frequent use in the passes. */
	HBBR_API HBBR_INLINE static int GetCurrentFrameIndex() {
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

	//ms
	HBBR_API HBBR_INLINE double GetFrameRate()const {
		return _frameRate;
	}

	//s
	HBBR_API HBBR_INLINE double GetFrameRateS()const {
		return _frameRate/1000.0f;
	}

	HBBR_API HBBR_INLINE bool IsWorldValid()const{
		if (_world)
			return true;
		else
			return false;
	}

	HBBR_API HBBR_INLINE class World* GetWorld() {
		if (IsWorldValid()){
			return _world.get();
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

	HBBR_API void LoadWorld(HString worldNameOrAssetPath);

	HBBR_API void CreateEmptyWorld();

	//获取游戏时间(秒)
	HBBR_INLINE const double GetGameTime()
	{
		return _gameTime.End_s();
	}

	/* 帧渲染函数 */
	HBBR_API void Render();

	HBBR_API void RendererResize(uint32_t w,uint32_t h);

	HBBR_API void Release();

	void Init();

	//Renderer map
	static std::map<HString, VulkanRenderer*> _renderers;

	VkSurfaceCapabilitiesKHR _surfaceCapabilities{};

	//spwan new world Callback
	HBBR_API HBBR_INLINE  void AddSpawnNewWorldCallBack( std::function<void(std::weak_ptr<World>)> func)
	{
		_spwanNewWorld.push_back(func);
	}

private:

	void SetupPassUniformBuffer();

	bool Resizing(bool bForce = false);

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

	static uint32_t _currentFrameIndex;

	uint32_t _maxSwapchainImages = 0;

	bool _bRendererRelease;

	bool _bInit;

	bool _bResize;

	std::vector<VkCommandBuffer> _cmdBuf;

	SDL_Window* _windowHandle = nullptr;

	//Pass Uniform
	PassUniformBuffer _passUniformBuffer;

	//Passes
	std::unique_ptr<class PassManager> _passManager;

	//World
	std::shared_ptr<class World> _world;

	HTime _frameTime;

	HTime _gameTime;

	double _frameRate;

	std::vector<std::function<void()>> _renderThreadFuncsOnce;

	std::vector<std::function<void()>> _renderThreadFuncs;

	bool _bIsInGame;

	std::vector<std::function<void(std::weak_ptr<World>)>> _spwanNewWorld;
};