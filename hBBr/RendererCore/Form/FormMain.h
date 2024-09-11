#pragma once
#include "Common.h"
#include "VulkanManager.h"
#include "FileSystem.h"
#include "HTime.h"

struct VulkanForm
{
	HString name;
	SDL_Window* window = nullptr;
	class VulkanSwapchain* swapchain = nullptr;
	bool bMinimized = false;
	bool bStopRender = false;
	std::vector<struct ImGuiContext*> imguiContents;
	std::vector<std::function<void(VulkanForm*)>> closeCallbacks;
	~VulkanForm();
};

typedef void (*FormDropFun)(VulkanForm* form,  HString file);

class VulkanApp
{
	friend class HInput;
public:
	/* 初始化Vulkan manager 和 第一个Vulkan渲染窗口
		@bCustomRenderLoop :是否开启自循环
		@bEnableDebug :是否开启Debug layer
		@return:如果bCustomRenderLoop为false,则返回这个vulkan渲染窗口
	*/
	HBBR_API static VulkanForm* InitVulkanManager(bool bCustomRenderLoop,bool bEnableDebug = false,void* parent = nullptr);
	HBBR_API static void DeInitVulkanManager();
	HBBR_API static bool UpdateForm();
	HBBR_API static void UpdateRender();
	HBBR_API static VulkanForm* CreateNewWindow(int x = 0, int y = 0, int w = 512, int h = 512, const char* title = "Renderer",bool bCreateRenderer = false ,void* parent = nullptr);
	HBBR_API static void CreateRenderer(VulkanForm* form);
	HBBR_API static bool IsWindowFocus(SDL_Window* windowHandle);
	HBBR_API static std::vector<VulkanForm*>& GetForms() { return _forms; }
	HBBR_API static void RemoveWindow(VulkanForm* form);
	HBBR_API static void ResizeWindow(SDL_Window* form, int w, int h);
	HBBR_API static void SetWindowPos(SDL_Window* form, int x, int y);
	HBBR_API static void* GetWindowHandle(SDL_Window* window);
	HBBR_API static void GetWindowSize(SDL_Window* window, int& w, int& h);
	HBBR_API static void GetWindowPos(SDL_Window* window, int& x, int& y);
	HBBR_API static inline VulkanForm* GetMainForm() { return _mainForm; }
	HBBR_API static void SetFocusForm(VulkanForm* form);
	HBBR_API static VulkanForm* GetFocusForm() { return _focusForm; }
	HBBR_API static void SetWindowVisible(SDL_Window* window, bool bShow);
	HBBR_API static void SetWindowTitle(SDL_Window* form, HString newTitle);
	HBBR_API static void AppQuit();
	HBBR_API static void RecompileAllShader();
	HBBR_API static void AddDropCallback(FormDropFun func)
	{
		_dropFuns.push_back(func);
	}
	//ms
	HBBR_API HBBR_INLINE static double GetFrameRate() {
		return _frameRate;
	}
	//s
	HBBR_API HBBR_INLINE static double GetFrameRateS() {
		return _frameRate / 1000.0f;
	}
	//获取游戏时间(秒)
	HBBR_INLINE static const double GetGameTime()
	{
		return _gameTime.End_s();
	}

#if IS_EDITOR
	static std::function<void()> _editorVulkanInit;
	HBBR_API static void SetEditorVulkanInit(std::function<void()> func);
#endif

	static std::vector<FormDropFun>& GetDropCallbacks() { return _dropFuns; }

private:

	//Callbacks
	static std::vector<FormDropFun> _dropFuns;

	static VulkanForm* _focusForm;

	static bool _bFocusQuit;

	static bool _bRecompilerShaders;

	static std::vector<VulkanForm*> _forms;

	static VulkanForm* _mainForm;

	static HTime _frameTime;

	static HTime _gameTime;

	static double _frameRate;

};

