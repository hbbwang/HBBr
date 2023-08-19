#include "FormMain.h"
#include "Shader.h"
#include "Pipeline.h"
#include "HInput.h"
#include "VulkanRenderer.h"
#if IS_EDITOR
#include "ShaderCompiler.h"
#endif

//#if _DEBUG
//#include "include/vld.h"
//#pragma comment(lib ,"vld.lib")
//#endif

std::vector<VulkanForm> VulkanApp::_forms;
VulkanForm* VulkanApp::_mainForm;
void* VulkanApp::_focusWindow = NULL;
std::vector<FormDropFun> VulkanApp::_dropFuns;

void ResizeCallBack(GLFWwindow* window, int width, int height)
{
	auto it = std::find_if(VulkanApp::GetForms().begin(),VulkanApp::GetForms().end(), [window](VulkanForm& glfw)
	{
		return glfw.window == window;
	});
	if (it != VulkanApp::GetForms().end() && it->renderer)
	{
		it->renderer->RendererResize((uint32_t)width, (uint32_t)height);
	}
}

void CloseCallBack(GLFWwindow* window)
{
	auto it = std::find_if(VulkanApp::GetForms().begin(), VulkanApp::GetForms().end(), [window](VulkanForm& glfw)
	{
		return glfw.window == window;
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

void FocusCallBack(GLFWwindow* window, int focused)
{
	if (focused == 1)
		VulkanApp::_focusWindow = window;
	else
		VulkanApp::_focusWindow = NULL;
}

void KeyBoardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	HInput::KeyProcess((void*)window , (KeyCode)key, (KeyMod)mods, (Action)action);
}

void MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	glfwFocusWindow(window);
}

void CursorPosCallBack(GLFWwindow* window, double xpos, double ypos)
{

}

void CursorEnterCallBack(GLFWwindow* window, int entered)
{

}

void ScrollCallBack(GLFWwindow* window, double xoffset, double yoffset)
{

}

void DropCallBack(GLFWwindow* window, int path_count, const char* paths[])
{
	for (auto& i : VulkanApp::_dropFuns)
	{
		i(path_count, paths);
	}
}

void JoystickCallBack(int jid, int event)
{

}

void MonitorCallBack(GLFWmonitor* monitor, int event)
{

}

VulkanForm* VulkanApp::InitVulkanManager(bool bCustomRenderLoop , bool bEnableDebug)
{
	//must be successful.
	if (!glfwInit())
	{
		MessageOut("Init glfw failed.", true, true, "255,0,0");
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	VulkanManager::InitManager(bEnableDebug);
#if IS_EDITOR
	Shaderc::ShaderCompiler::CompileAllShaders(FileSystem::GetShaderIncludeAbsPath().c_str());
#endif
	Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());

	//Create Main Window
	auto win = CreateNewWindow(128, 128, "MainRenderer", true);

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
				glfwSetWindowShouldClose(_forms[i].window, true);
			}
		}
		_forms.clear();
	}
	Shader::DestroyAllShaderCache();
	PipelineManager::ClearPipelineObjects();
	VulkanManager::ReleaseManager();
	glfwTerminate();
}

bool VulkanApp::UpdateForm()
{
	bool quit = true;
	glfwPollEvents();
	for (int i = 0; i < _forms.size(); i++)
	{
		if (glfwWindowShouldClose(_forms[i].window))
		{
			RemoveWindow(&_forms[i]);
			i = i - 1;
			return true;
		}
		else if (_forms[i].renderer)
		{
			quit = false;
			_forms[i].renderer->Render();
		}
	}
	if (quit)
	{
		return false;
	}
	else
	{
		HInput::ClearInput();
	}
	return true;
}

VulkanForm* VulkanApp::CreateNewWindow(uint32_t w, uint32_t h , const char* title, bool bCreateRenderer)
{
	GLFWwindow* window = glfwCreateWindow(w, h, title, nullptr, nullptr);
	if (!window)
	{
		MessageOut("Create glfw window failed.", true, true, "255,0,0");
		glfwTerminate();
	}
	//set event
	glfwSetWindowSizeCallback(window, ResizeCallBack);
	glfwSetWindowCloseCallback(window, CloseCallBack);
	glfwSetWindowFocusCallback(window,FocusCallBack);
	glfwSetKeyCallback(window,KeyBoardCallBack);
	glfwSetMouseButtonCallback(window,MouseButtonCallBack);
	glfwSetCursorPosCallback(window,CursorPosCallBack);
	glfwSetCursorEnterCallback(window,CursorEnterCallBack);
	glfwSetScrollCallback(window,ScrollCallBack);
	glfwSetDropCallback(window,DropCallBack);

	VulkanForm newGLFW = {};
	newGLFW.window = window;
	newGLFW.name = title;
	if (bCreateRenderer)
	{
		newGLFW.renderer = new VulkanRenderer((void*)newGLFW.window , title);
	}
	_forms.push_back(newGLFW);
	return &_forms[_forms.size() - 1];
}

bool VulkanApp::IsWindowFocus(void* windowHandle)
{
	if (_focusWindow != NULL)
	{
		return  _focusWindow == windowHandle;
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
	if(form && form->window)
		glfwSetWindowSize(form->window, (int)w, (int)h);
}

void VulkanApp::SetWindowPos(VulkanForm* form, uint32_t x, uint32_t y)
{
	if (form && form->window)
		glfwSetWindowPos(form->window, (int)x, (int)y);
}

void* VulkanApp::GetWindowHandle(VulkanForm* form)
{
	if (form && form->window)
	{
		#if defined(_WIN32)
		return (void*)glfwGetWin32Window((GLFWwindow*)form->window);
		#endif
	}
	return NULL;
}

void VulkanApp::SetFormFocus(VulkanForm* form)
{
	if (form && form->window)
	{
		glfwFocusWindow((GLFWwindow*)form->window);
	}
}

void VulkanApp::SetFormVisiable(VulkanForm* form, bool bShow)
{
	if (form && form->window)
	{
		if (bShow)
		{
			glfwShowWindow(form->window);
		}
		else
		{
			glfwHideWindow(form->window);
		}
	}
}

#if IS_GAME
int main(int argc, char* argv[])
{
	//Enable custom loop
	VulkanApp::InitVulkanManager(true, true);
	VulkanApp::DeInitVulkanManager();
	return 0;
}
#else

#endif