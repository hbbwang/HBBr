#include "./Core/VulkanManager.h"
#include "./Core/VulkanRenderer.h"
#include "./Form/FormMain.h"
#include "./Core/Shader.h"
#include "./Core/Pipeline.h"
#include "./Common/HInput.h"
#include "./Asset/Texture2D.h"
#include "./Asset/World.h"
#include "./Core/RendererConfig.h"
#include "./Asset/Material.h"
#if IS_EDITOR
#include "ShaderCompiler.h"
#include "Imgui/backends/imgui_impl_sdl3.h"
#endif
#include "./Asset/ContentManager.h"
#include "GLFWInclude.h"
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
	if (it != VulkanApp::GetForms().end() && (*it)->renderer)
	{
		(*it)->renderer->RendererResize((uint32_t)width, (uint32_t)height);
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
		if ((*it)->renderer)
		{
			(*it)->renderer->Release();
			(*it)->renderer = nullptr;
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
		SDL_SetWindowInputFocus(form->window);
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
	CallStack();

	//must be successful.
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		MessageOut("Init sdl3 failed.", true, true, "255,0,0");
	}
	
	if (SDL_Vulkan_LoadLibrary(nullptr) == -1)
	{
		MessageOut(SDL_GetError(), true, true, "255,0,0");
	}

	//Set sdl hints
	SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "0");
	SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1");//SDL_WINDOW_VULKAN

#if __ANDROID__
	Android_Init();
#endif

	//Init Vulkan Manager
	VulkanManager::InitManager(bEnableDebug);

	//Import font
	//HString ttfFontPath = FileSystem::GetAssetAbsPath() + "Font/msyhl.ttc";
	HString ttfFontPath = GetRendererConfig("Default", "Font") ;
	FileSystem::CorrectionPath(ttfFontPath);
	HString outFontTexturePath = GetRendererConfig("Default", "FontTexture") ;
	outFontTexturePath = FileSystem::GetRelativePath(outFontTexturePath.c_str());
	outFontTexturePath = FileSystem::GetProgramPath() + outFontTexturePath;
	FileSystem::CorrectionPath(outFontTexturePath);
	//Texture2D::CreateFontTexture(ttfFontPath, outFontTexturePath, true, 16U, 4096U);

	Texture2D::GlobalInitialize();

	World::CollectWorlds();

	//Create Main Window
	auto win = CreateNewWindow(128, 128, "MainRenderer", false, parent);

#if IS_EDITOR
	Shaderc::ShaderCompiler::CompileAllShaders(FileSystem::GetShaderIncludeAbsPath().c_str());
#endif
	Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());
	//Init Content Manager
	ContentManager::Get();

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
		for (int i = 0; i < _forms.size(); i++)
		{
			if (_forms[i]->renderer)
			{
				_forms[i]->renderer->Release();
				_forms[i]->renderer = nullptr;
			}
			if (_forms[i]->window)
			{
				SDL_DestroyWindow(_forms[i]->window);
			}
			delete _forms[i];
			_forms[i] = nullptr;
		}
		_forms.clear();
	}
	//销毁所有世界
	World::GetWorlds().clear();
	Shader::DestroyAllShaderCache();
	PipelineManager::ClearPipelineObjects();
	ContentManager::Get()->Release();
	Texture2D::GlobalRelease();
	VulkanManager::ReleaseManager();
	//保存配置文件
	SaveRendererConfig();
	SDL_Quit();
}

//DISABLE_CODE_OPTIMIZE
bool VulkanApp::UpdateForm()
{
	if (_mainForm == nullptr || _mainForm->renderer == nullptr)
	{
		return false;
	}
	bool bQuit = false;
	SDL_Event event;
	bool bStopRender = false; 
	while (SDL_PollEvent(&event))
	{
		VulkanForm* winForm = nullptr;
		for (auto& i : _forms)
		{
			Uint32 flags = SDL_GetWindowFlags(i->window);
			if (flags & SDL_WINDOW_INPUT_FOCUS || flags & SDL_WINDOW_MOUSE_FOCUS)
			{
				winForm = i;
			}
		}
		if (winForm == nullptr)
		{
			winForm = _forms[0];
		}

#ifdef IS_EDITOR
		ImGui_ImplSDL3_ProcessEvent(&event); 
#endif

		//Get window form
		switch (event.type)
		{
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED: //窗口关闭事件
		{
			CloseCallBack(winForm->window);
			SDL_DestroyWindow(winForm->window);
			RemoveWindow(winForm);
			if (winForm == GetFocusForm())
			{
				FocusCallBack(winForm, 0);
			}
			break;
		}
		case SDL_EVENT_WINDOW_TAKE_FOCUS:
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
			ResizeCallBack(win, event.window.data1, event.window.data2);
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
			for (auto& func : VulkanRenderer::_mouse_inputs)
			{
				func.second(winForm->renderer, (MouseButton)event.button.button, (Action)event.button.state);
			}
			break;
		}
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		{
			KeyBoardCallBack(winForm, event.key.keysym.sym, event.key.keysym.scancode, event.key.state, event.key.keysym.mod);
			for (auto& func :VulkanRenderer::_key_inputs)
			{
				func.second(winForm->renderer, (KeyCode)event.key.keysym.sym, (KeyMod)event.key.keysym.mod, (Action)event.key.state);
			}
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL:
			break;
		case SDL_EVENT_DROP_FILE:
		{
			char* file = event.drop.file;
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
				SDL_free(file);
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
			i.second->NeedToReload();
			Material::LoadAsset(i.second->guid);
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
		if(w->renderer != nullptr && !w->bMinimized)
			w->renderer->Render();
	}
}
//ENABLE_CODE_OPTIMIZE

VulkanForm* VulkanApp::CreateNewWindow(uint32_t w, uint32_t h , const char* title, bool bCreateRenderer, void* parent)
{
	SDL_Window* window = nullptr;
	if (parent != nullptr)
	{
		window = SDL_CreateWindowFrom(parent);
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
		MessageOut((HString("Create sdl3 window failed : ")+ SDL_GetError()).c_str(), true, true, "255,0,0");
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
	return _forms[_forms.size() - 1];
}

void VulkanApp::CreateRenderer(VulkanForm* form)
{
	if (form != nullptr)
	{
		form->renderer = new VulkanRenderer(form->window, form->name.c_str());
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
	if (form != nullptr && form != nullptr)
	{
		auto window = form->window;
		auto it = std::remove_if(_forms.begin(), _forms.end(), [window](VulkanForm* &glfw) {
			return window == glfw->window;
			});
		if (it != _forms.end())
		{
			delete (*it);
			_forms.erase(it);
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

void* VulkanApp::GetWindowHandle(VulkanForm* form)
{
	if (form && form->window)
	{
		#if defined(_WIN32)
		SDL_SysWMinfo wmInfo;
		SDL_GetWindowWMInfo(form->window, &wmInfo , SDL_SYSWM_CURRENT_VERSION);
		return (void*)wmInfo.info.win.window;
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

#if defined(IS_GAME)

int main(int argc, char* argv[])
{
    //SDL_ShowSimpleMessageBox(0,"","",nullptr);
    //ConsoleDebug::CreateConsole("");
	//Enable custom loop
	VulkanApp::InitVulkanManager(true, true);
	VulkanApp::DeInitVulkanManager();
	return 0;
}

#else

#endif //defined(IS_GAME)
