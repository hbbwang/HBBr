#include "./Form/FormMain.h"
#include "./Core/Shader.h"
#include "./Core/Pipeline.h"
#include "./Common/HInput.h"
#include "./Core/VulkanRenderer.h"
#if IS_EDITOR
#include "ShaderCompiler.h"
#endif
#include "./Resource/ContentManager.h"

//#if _DEBUG
//#include "include/vld.h"
//#pragma comment(lib ,"vld.lib")
//#endif

std::vector<VulkanForm> VulkanApp::_forms;
VulkanForm* VulkanApp::_mainForm;
VulkanForm* VulkanApp::_focusForm = NULL;
std::vector<FormDropFun> VulkanApp::_dropFuns;
bool VulkanApp::_bFocusQuit = false;
void ResizeCallBack(SDL_Window* window, int width, int height)
{
	auto it = std::find_if(VulkanApp::GetForms().begin(),VulkanApp::GetForms().end(), [window](VulkanForm& form)
	{
		return form.window == window;
	});
	if (it != VulkanApp::GetForms().end() && it->renderer)
	{
		it->renderer->RendererResize((uint32_t)width, (uint32_t)height);
	}
}

void CloseCallBack(SDL_Window* window)
{
	auto it = std::find_if(VulkanApp::GetForms().begin(), VulkanApp::GetForms().end(), [window](VulkanForm& form)
	{
		return form.window == window;
	});
	if (it != VulkanApp::GetForms().end())
	{
		if (it->renderer)
		{
			it->renderer->Release();
			it->renderer = NULL;
		}
	}
}

void FocusCallBack(VulkanForm* window, int focused)
{
	if (focused == 1)
		VulkanApp::_focusForm = window;
	else
		VulkanApp::_focusForm = NULL;
}

void KeyBoardCallBack(SDL_Window* window, SDL_Keycode key, int scancode, int action, int mods)
{
	HInput::KeyProcess((void*)window , (KeyCode)key, (KeyMod)mods, (Action)action);
}

void MouseButtonCallBack(SDL_Window* window, int button, int action)
{
	SDL_SetWindowInputFocus(window);
	SDL_SetWindowFocusable(window, SDL_TRUE);

	//for (int i = 0; i < VulkanApp::GetForms().size(); ++i)
	//{
	//	if (VulkanApp::GetForms()[i].window == window)
	//	{
	//		FocusCallBack(&VulkanApp::GetForms()[i], 1);
	//		break;
	//	}
	//}
	HInput::MouseProcess((void*)window, (MouseButton)button,(Action)action);
}

void ScrollCallBack(SDL_Window* window, double xoffset, double yoffset)
{

}

void DropCallBack(SDL_Window* window, int path_count, const char* paths[])
{
	for (auto& i : VulkanApp::_dropFuns)
	{
		i(path_count, paths);
	}
}

VulkanForm* VulkanApp::InitVulkanManager(bool bCustomRenderLoop , bool bEnableDebug, void* parent)
{
	//must be successful.
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		MessageOut("Init sdl2 failed.", true, true, "255,0,0");
	}

	if (SDL_Vulkan_LoadLibrary(NULL) == -1)
	{
		MessageOut(SDL_GetError(), true, true, "255,0,0");
	}
	SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, "HBBr msg", "Start InitManager ", NULL);
	VulkanManager::InitManager(bEnableDebug);
#if IS_EDITOR
	Shaderc::ShaderCompiler::CompileAllShaders(FileSystem::GetShaderIncludeAbsPath().c_str());
#endif
	Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());
	ContentManager::Get();

	//Set sdl hints
	SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1");//SDL_WINDOW_VULKAN

	//Create Main Window
	auto win = CreateNewWindow(128, 128, "MainRenderer", true, parent);

	//Try Refresh focus
	SetFormVisiable(win, false);
	SetFormVisiable(win, true);

	_mainForm = win;

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
	return NULL;
}

void VulkanApp::DeInitVulkanManager()
{
	if (_forms.size() > 0)
	{
		for (int i = 0; i < _forms.size(); i++)
		{
			if (_forms[i].renderer)
			{
				_forms[i].renderer->Release();
				_forms[i].renderer = NULL;
			}
			if (_forms[i].window)
			{
				SDL_DestroyWindow(_forms[i].window);
			}
		}
		_forms.clear();
	}
	Shader::DestroyAllShaderCache();
	PipelineManager::ClearPipelineObjects();
	VulkanManager::ReleaseManager();
	//ContentManager::Get()->Release();
	SDL_Quit();
}

//DISABLE_CODE_OPTIMIZE
bool VulkanApp::UpdateForm()
{
	bool bQuit = false;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		auto win = SDL_GetWindowFromID(event.window.windowID);
		auto winForm = std::find_if(_forms.begin(), _forms.end(), [win](VulkanForm& form) {
			return form.window == win;
		});
		switch (event.type) 
		{
			//case 0x200://窗口事件,SDL3开始不再需要这个了
				//WindowEvent(event);
				//break;
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED: //窗口关闭事件
				CloseCallBack(win);
				SDL_DestroyWindow(win);
				RemoveWindow(&(*winForm));
				break;
			case SDL_EVENT_WINDOW_TAKE_FOCUS:
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				FocusCallBack(&(*winForm), 1);
				break;
			case SDL_EVENT_WINDOW_FOCUS_LOST:
				FocusCallBack(&(*winForm), 0);
				break;
			case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			case SDL_EVENT_WINDOW_RESIZED:
				ResizeCallBack(win, event.window.data1, event.window.data2);
				break;
			case SDL_EVENT_QUIT://窗口完全关闭,SDL即将退出
				bQuit = true;
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				MouseButtonCallBack(win, event.button.button, event.button.state);
				break;
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
				KeyBoardCallBack(win, event.key.keysym.sym, event.key.keysym.scancode, event.key.state, event.key.keysym.mod);
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				break;
		}
	}

	if (_bFocusQuit || bQuit)
	{
		for (auto w : _forms)
		{
			CloseCallBack(w.window);
			SDL_DestroyWindow(w.window);
			RemoveWindow(&w);
		}
		MessageOut("SDL quit.", false, false, "255,255,255");
		bQuit = true;
	}
	else
	{
		UpdateRender();
	}

	return !bQuit;
}

void VulkanApp::UpdateRender()
{
	for (auto w : _forms)
	{
		w.renderer->Render();
	}
	HInput::ClearInput();
}
//ENABLE_CODE_OPTIMIZE

VulkanForm* VulkanApp::CreateNewWindow(uint32_t w, uint32_t h , const char* title, bool bCreateRenderer, void* parent)
{
	SDL_Window* window = NULL;
	if (parent != NULL)
	{
		window = SDL_CreateWindowFrom(parent);
	}

	if(!window)
	{
		window = SDL_CreateWindow(title, w, h,
			SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	}

	if (!window)
	{
		MessageOut("Create sdl2 window failed.", true, true, "255,0,0");
		SDL_Quit();
	}

	VulkanForm newForm = {};
	newForm.window = window;
	newForm.name = title;
	if (bCreateRenderer)
	{
		newForm.renderer = new VulkanRenderer( window , title);
	}
	_forms.push_back(newForm);
	return &_forms[_forms.size() - 1];
}

bool VulkanApp::IsWindowFocus(SDL_Window* windowHandle)
{
	if (_focusForm != NULL)
	{
		return  _focusForm->window == windowHandle;
	}
	return false;
}

void VulkanApp::RemoveWindow(VulkanForm* form)
{
	auto window = form->window;
	auto it = std::remove_if(_forms.begin(), _forms.end(), [window](VulkanForm& glfw) {
		return window == glfw.window;
		});
	if (it != _forms.end())
	{
		_forms.erase(it);
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
	return NULL;
}

void VulkanApp::SetFormFocus(VulkanForm* form)
{
	if (form && form->window)
	{
		SDL_SetWindowInputFocus(form->window);
		SDL_SetWindowFocusable(form->window, SDL_TRUE);
		FocusCallBack(form, 1);
	}
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

#if IS_GAME

int main(int argc, char* argv[])
{
	//Enable custom loop
	VulkanApp::InitVulkanManager(true, true);
	VulkanApp::DeInitVulkanManager();
	return 0;
}

int SDL_main(int argc, char* argv[])
{
	return main(argc, argv);
}

#else

#endif
