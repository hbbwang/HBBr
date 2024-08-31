
#include "./Core/VulkanManager.h"
#include "./Core/VulkanRenderer.h"
#include "./Core/VulkanSwapchain.h"
#include "./Form/FormMain.h"
#include "./Core/Shader.h"
#include "./Core/Pipeline.h"
#include "./Common/HInput.h"
#include "./Asset/Texture2D.h"
#include "./Asset/World.h"
#include "./Core/RendererConfig.h"
#include "./Asset/Material.h"
#include "VulkanObjectManager.h"

#if IS_EDITOR
#include "ShaderCompiler.h"
#endif

#if ENABLE_IMGUI
#include "Imgui/backends/imgui_impl_sdl3.h"
#endif

#include "./Asset/ContentManager.h"
#include "SDLInclude.h"
#include "ConsoleDebug.h"
#if defined(__ANDROID__)
#ifndef IS_GAME
#define IS_GAME 1
#endif
#endif

#include "main.h"

//#if _DEBUG
//#include "include/vld.h"
//#pragma comment(lib ,"vld.lib")
//#endif

#if IS_EDITOR
std::function<void()>VulkanApp::_editorVulkanInit = []() {};
#endif
std::vector<VulkanForm*> VulkanApp::_forms;
VulkanForm* VulkanApp::_mainForm = nullptr;
VulkanForm* VulkanApp::_focusForm = nullptr;
std::vector<FormDropFun> VulkanApp::_dropFuns;
bool VulkanApp::_bFocusQuit = false;
bool VulkanApp::_bRecompilerShaders = false;
HTime VulkanApp::_frameTime;
HTime VulkanApp::_gameTime;
double VulkanApp::_frameRate = 0 ;

void ResizeCallBack(SDL_Window* window, int width, int height)
{
	auto it = std::find_if(VulkanApp::GetForms().begin(),VulkanApp::GetForms().end(), [window](VulkanForm*&form)
	{
		return form->window == window;
	});
	if (it != VulkanApp::GetForms().end() && (*it)->swapchain)
	{
		(*it)->swapchain->ResizeBuffer();
	}
}

void CloseCallBack(SDL_Window* window)
{
	auto it = std::find_if(VulkanApp::GetForms().begin(), VulkanApp::GetForms().end(), [window](VulkanForm*&form)
	{
		return form->window == window;
	});
	if (it != VulkanApp::GetForms().end())
	{
		if ((*it)->swapchain)
		{
			(*it)->swapchain->Release();
			(*it)->swapchain = nullptr;
		}
	}
}

void FocusCallBack(VulkanForm* form, int focused)
{
	if (form)
	{
		if (focused == 1)
			VulkanApp::SetFocusForm(form);
		else
			VulkanApp::SetFocusForm(nullptr);
	}
}

void KeyBoardCallBack(VulkanForm* form, SDL_Keycode key, int scancode, int action, int mods)
{

}

void MouseButtonCallBack(VulkanForm* form, int button, int action)
{
	if (form)
	{
		SDL_SetWindowFocusable(form->window, SDL_TRUE);
	}
}

void ScrollCallBack(SDL_Window* window, double xoffset, double yoffset)
{

}

void DropCallBack(VulkanForm* form, const char* file)
{
	for (auto& i : VulkanApp::GetDropCallbacks())
	{
		i(form, HString(file));
	}
}
#if __ANDROID__
void Android_Init()
{

}
#endif

VulkanForm* VulkanApp::InitVulkanManager(bool bCustomRenderLoop , bool bEnableDebug, void* parent)
{
#if _WIN32
	SetConsoleOutputCP(65001); // 设置控制台输出代码页为 UTF-8
#endif

	CallStack();

	//must be successful.
    if (SDL_Init(
		 SDL_INIT_TIMER 
		| SDL_INIT_AUDIO 
		| SDL_INIT_VIDEO 
		| SDL_INIT_JOYSTICK 
		| SDL_INIT_HAPTIC 
		| SDL_INIT_GAMEPAD 
		| SDL_INIT_EVENTS 
		| SDL_INIT_SENSOR  
		//| SDL_INIT_CAMERA
	) == -1)
	{
		MessageOut("Init sdl3 failed.", true, true, "255,0,0");
	}
	
	if (SDL_Vulkan_LoadLibrary(nullptr) == -1)
	{
		MessageOut(SDL_GetError(), true, true, "255,0,0");
	}

	//Set sdl hints
	SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "0");

#if __ANDROID__
	Android_Init();
#endif

	//Init Vulkan Manager
	VulkanManager::InitManager(bEnableDebug);

	Texture2D::GlobalInitialize();

	//Create Main Window
	auto win = CreateNewWindow(-0, -0, 130, 130, "MainRenderer", false, parent);

	SDL_HideWindow(win->window);

#if IS_EDITOR
	VulkanApp::_editorVulkanInit();

	Shaderc::ShaderCompiler::CompileAllShaders(FileSystem::GetShaderIncludeAbsPath().c_str());
#endif
	Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());
	
	//Init Content Manager
	ContentManager::Get();

	//Init Pipeline Common Objects
	PipelineManager::GlobalInit();

#if __ANDROID__
	_Sleep(200);//延迟一点创建
#endif

	CreateRenderer(win);

	_mainForm = win;

	//Start GameTime
	_gameTime.Start();

	if (bCustomRenderLoop)
	{
		while (UpdateForm())
		{
		}
	}
	else
	{
		return win;
	}
	return nullptr;
}

void VulkanApp::DeInitVulkanManager()
{
	if (_forms.size() > 0)
	{
		for (auto& i : _forms)
		{
			if (i->swapchain)
			{
				i->swapchain->Release();
				i->swapchain = nullptr;
			}
			if (i->window)
			{
				SDL_DestroyWindow(i->window);
			}
			delete i;
			i = nullptr;
		}
		_forms.clear();
	}
	Shader::DestroyAllShaderCache();
	PipelineManager::ClearPipelineObjects();
	PipelineManager::GlobalRelease();
	ContentManager::Get()->Release();
	Texture2D::GlobalRelease();
	PrimitiveProxy::ClearAll();
	VulkanObjectManager::Get()->Release();
	VulkanManager::ReleaseManager();
	//保存配置文件
	SaveRendererConfig();
	SDL_Quit();
}

//DISABLE_CODE_OPTIMIZE
bool VulkanApp::UpdateForm()
{
	if (_mainForm == nullptr || _mainForm->swapchain == nullptr)
	{
		return false;
	}

	bool bQuit = false;
	SDL_Event event;
	bool bStopRender = false; 
	while (SDL_PollEvent(&event))
	{
		VulkanForm* winForm = nullptr;
		int windowIndex = 0;
		for (auto& i : _forms)
		{
			SDL_WindowFlags flags = SDL_GetWindowFlags(i->window);
			if (flags & SDL_WINDOW_INPUT_FOCUS || flags & SDL_WINDOW_MOUSE_FOCUS)
			{
				winForm = i;
				break;
			}
			windowIndex++;
		}

		//窗口数量 == 0, 直接结束 
		if (_forms.size() <= 0)
			return false;

		if (winForm == nullptr)
		{
			winForm = *_forms.begin();
			windowIndex = 0;
		}
		#if ENABLE_IMGUI
		for(auto& i: winForm->imguiContents)
		{
			if (i)
			{
				ImGui::SetCurrentContext(i);
				ImGui_ImplSDL3_ProcessEvent(&event);
			}
		}
		#endif

		//Get window form
		switch (event.type)
		{
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED: //窗口关闭事件
		{
			for (auto& ccb : winForm->closeCallbacks)
			{
				ccb(winForm);
			}
			CloseCallBack(winForm->window);
			SDL_DestroyWindow(winForm->window);
			RemoveWindow(winForm);
			if (winForm == GetFocusForm())
			{
				FocusCallBack(winForm, 0);
			}

			//窗口数量 == 0, 直接结束 
			if (_forms.size() <= 0)
				return false;

			break;
		}
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		{
			FocusCallBack(winForm, 1);
			break;
		}
		case SDL_EVENT_WINDOW_FOCUS_LOST:
		{
			FocusCallBack(winForm, 0);
#ifdef __ANDROID__
			bStopRender = true;
			ResizeWindow(winForm, 1, 1);
#endif
			break;
		}
		//case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
		//case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
		//case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:

#if !IS_EDITOR //Editor下咱们窗口缩放不走SDL的逻辑
		case SDL_EVENT_WINDOW_RESIZED:
		{
			ResizeCallBack(winForm->window, event.window.data1, event.window.data2);
			break;
		}
#endif

		case SDL_EVENT_QUIT://窗口完全关闭,SDL即将退出
		{
			bQuit = true;
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			MouseButtonCallBack(winForm, event.button.button, event.button.state);

			for (auto& renderer : winForm->swapchain->GetRenderers())
			{
				for (auto& func : renderer.second->_mouse_inputs)
				{
					func.second(renderer.second, (MouseButton)event.button.button, (Action)event.button.state);
				}	
			}
			break;
		}
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		{
			KeyBoardCallBack(winForm, event.key.key, event.key.scancode, event.key.state, event.key.mod);
			for (auto& renderer : winForm->swapchain->GetRenderers())
			{
				for (auto& func : renderer.second->_key_inputs)
				{
					func.second(renderer.second, (KeyCode)event.key.key, (KeyMod)event.key.mod, (Action)event.key.state);
				}
			}
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL:
			break;
		case SDL_EVENT_DROP_FILE:
		{
			auto file = event.drop.data;
			if (file)
			{
				auto win = SDL_GetWindowFromID(event.drop.windowID);
				auto winFormIt = std::find_if(_forms.begin(), _forms.end(), [win](VulkanForm*& form) {
					return form->window == win;
					});
				if (*winFormIt)
				{
					DropCallBack(winForm, file);
				}
			}
			break;
		}
		case SDL_EVENT_WINDOW_EXPOSED:
		case SDL_EVENT_WINDOW_SHOWN:
		{
			if (winForm)
			{
				winForm->bMinimized = false;
			}
			break;
		}
		case SDL_EVENT_WINDOW_MINIMIZED:
		case SDL_EVENT_WINDOW_HIDDEN:
		{
			if (winForm)
			{
				winForm->bMinimized = true;
			}
			bStopRender = true;
			break;
		}
		case SDL_EVENT_FINGER_DOWN:
		{
			//SDL_ShowSimpleMessageBox(0, "", "(test)手指按下", nullptr);
			break;
		}
		}
	}
	if (_bFocusQuit || bQuit)
	{
		for (auto w : _forms)
		{
			CloseCallBack(w->window);
			SDL_DestroyWindow(w->window);
			RemoveWindow(w);
		}
		MessageOut("SDL quit.", false, false, "255,255,255");
		bQuit = true;
	}
	else if (_bRecompilerShaders)
	{
		_bRecompilerShaders = false;
		Shader::DestroyAllShaderCache();
		PipelineManager::ClearPipelineObjects();
#if IS_EDITOR
		Shaderc::ShaderCompiler::CompileAllShaders(FileSystem::GetShaderIncludeAbsPath().c_str());
#endif
		Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());
		//需要重新加载所有材质
		auto allMaterials = ContentManager::Get()->GetAssets(AssetType::Material);
		for (auto& i : allMaterials)
		{
			if (i.second->IsAssetLoad())
			{
				i.second->NeedToReload();
				Material::LoadAsset(i.second->guid);
			}
		}
	}
	else if (!bStopRender)
	{
		UpdateRender();
	}
	return !bQuit;
}

void VulkanApp::UpdateRender()
{
	_frameRate = _frameTime.FrameRate_ms();
	for (auto w : _forms)
	{
		Texture2D::GlobalUpdate();
		VulkanObjectManager::Get()->Update();
		if(w->swapchain != nullptr && !w->bMinimized && !w->bStopRender)
			w->swapchain->Update();
	}
}
//ENABLE_CODE_OPTIMIZE

VulkanForm* VulkanApp::CreateNewWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h , const char* title, bool bCreateRenderer, void* parent)
{
	SDL_Window* window = nullptr;
	if (parent != nullptr)
	{
		SDL_PropertiesID props;
		props = SDL_CreateProperties();
		SDL_SetNumberProperty(props, "width", h);
		SDL_SetNumberProperty(props, "height", w);
		SDL_SetNumberProperty(props, "x", x);
		SDL_SetNumberProperty(props, "y", y);
		SDL_SetStringProperty(props, "title", title); 
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, SDL_TRUE);
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, SDL_TRUE);	
		SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_PARENT_POINTER, parent);

		#if defined(__ANDROID__)
		SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, SDL_TRUE);
		#elif _WIN32
		//SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, parent);
		#endif

		window = SDL_CreateWindowWithProperties(props);

	}

	if(!window)
	{
		window = SDL_CreateWindow(title, w, h,
			SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
#if defined(__ANDROID__)
			| SDL_WINDOW_FULLSCREEN
#else
#endif
		);
	}

	if (!window)
	{
		MessageOut((HString("Create sdl3 window failed : ")+ SDL_GetError()), true, true, "255,0,0");
		SDL_Quit();
	}

	VulkanForm* newForm = new VulkanForm;
	newForm->window = window;
	newForm->name = title;
	if (bCreateRenderer)
	{
		CreateRenderer(newForm);
	}
	_forms.push_back(newForm);
	return _forms.back();
}

void VulkanApp::CreateRenderer(VulkanForm* form)
{
	//Create Swapchain
	if (form != nullptr && form->swapchain == nullptr)
	{
		form->swapchain = new VulkanSwapchain(form->window);
	}
	if (form != nullptr && form->swapchain != nullptr)
	{
		form->swapchain->CreateRenderer(form->name.c_str());
		//Try Refresh focus
		SetFormVisiable(form, false);
		SetFormVisiable(form, true);
	}
}

bool VulkanApp::IsWindowFocus(SDL_Window* windowHandle)
{
	if (GetFocusForm() != nullptr)
	{
		return  GetFocusForm()->window == windowHandle;
	}
	return false;
}

void VulkanApp::RemoveWindow(VulkanForm* form)
{
	if (form != nullptr && form->window != nullptr)
	{
		ConsoleDebug::printf_endl("%s Renderer Quit.", form->name.c_str());
		for (int i = 0; i < _forms.size(); i++)
		{
			if (_forms[i]->window == form->window)
			{
				auto form = _forms[i];
				_forms.erase(_forms.begin() + i);
				if (form->swapchain)
				{
					form->swapchain->Release();
					form->swapchain = nullptr;
				}
				delete form;
				form = nullptr;
				break;
			}
		}
	}
}

void VulkanApp::ResizeWindow(VulkanForm* form, uint32_t w, uint32_t h)
{
	if (w < 1 || h < 1)
	{
		return;
	}
	if (form && form->window)
		SDL_SetWindowSize(form->window, (int)w, (int)h);
}

void VulkanApp::SetWindowPos(VulkanForm* form, uint32_t x, uint32_t y)
{
	if (form && form->window)
		SDL_SetWindowPosition(form->window, (int)x, (int)y);
}

void* VulkanApp::GetWindowHandle(SDL_Window* window)
{
	if (window)
	{
		#if defined(SDL_PLATFORM_WIN32)
		HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
		return hwnd;
		//#elif defined(SDL_PLATFORM_MACOS)
		//NSWindow* nswindow = (__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
		//return nswindow;
		//#elif defined(SDL_PLATFORM_LINUX)
		//if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
		//	Display* xdisplay = (Display*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
		//	Window xwindow = (Window)SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
		//	if (xdisplay && xwindow) {
		//		...
		//	}
		//}
		//else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
		//	struct wl_display* display = (struct wl_display*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
		//	struct wl_surface* surface = (struct wl_surface*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
		//	if (display && surface) {
		//		...
		//	}
		//}
		//#elif defined(SDL_PLATFORM_IOS)
		//SDL_PropertiesID props = SDL_GetWindowProperties(window);
		//UIWindow* uiwindow = (__bridge UIWindow*)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
		//if (uiwindow) {
		//	GLuint framebuffer = (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER, 0);
		//	GLuint colorbuffer = (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_RENDERBUFFER_NUMBER, 0);
		//	GLuint resolveFramebuffer = (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_RESOLVE_FRAMEBUFFER_NUMBER, 0);
		//	...
		//}
		#endif
	}
	return nullptr;
}

void VulkanApp::SetFocusForm(VulkanForm* form)
{
	_focusForm = form;
}

void VulkanApp::SetFormVisiable(VulkanForm* form, bool bShow)
{
	if (form && form->window)
	{
		if (bShow)
		{
			SDL_ShowWindow(form->window);
		}
		else
		{
			SDL_HideWindow(form->window);
		}
	}
}

void VulkanApp::AppQuit()
{
	_bFocusQuit = true;
}

void VulkanApp::RecompileAllShader()
{
	_bRecompilerShaders = true;
}

#if IS_EDITOR
void VulkanApp::SetEditorVulkanInit(std::function<void()> func)
{
	_editorVulkanInit = func;
}
#endif

#if defined(IS_GAME)

//int main(int argc, char* argv[])
//{
//    //SDL_ShowSimpleMessageBox(0,"","",nullptr);
//    //ConsoleDebug::CreateConsole("");
//	//Enable custom loop
//	VulkanApp::InitVulkanManager(true, true);
//	VulkanApp::DeInitVulkanManager();
//	return 0;
//}

#else

#endif //defined(IS_GAME)
