#include "FormMain.h"
#include "Shader.h"
#include "Pipeline.h"
#include "HInput.h"
#if IS_EDITOR
#include "ShaderCompiler.h"
#endif

//#if _DEBUG
//#include "include/vld.h"
//#pragma comment(lib ,"vld.lib")
//#endif

std::vector<VulkanForm> VulkanApp::_glfwWindows;
VulkanForm* VulkanApp::_mainForm;
void* VulkanApp::_focusWindow = NULL;
std::vector<FormDropFun> VulkanApp::_dropFuns;

void GLFWResize(GLFWwindow* window, int width, int height)
{
	auto it = std::find_if(VulkanApp::GetWindows().begin(),VulkanApp::GetWindows().end(), [window](VulkanForm& glfw)
	{
		return glfw.window == window;
	});
	if (it != VulkanApp::GetWindows().end() && it->renderer)
	{
		it->renderer->RendererResize((uint32_t)width, (uint32_t)height);
	}
}

void GLFWClose(GLFWwindow* window)
{
	auto it = std::find_if(VulkanApp::GetWindows().begin(), VulkanApp::GetWindows().end(), [window](VulkanForm& glfw)
	{
		return glfw.window == window;
	});
	if (it != VulkanApp::GetWindows().end())
	{
		if (it->renderer)
		{
			it->renderer->Release();
			it->renderer = NULL;
		}
	}
}

void GLFWFocus(GLFWwindow* window, int focused)
{
	if (focused == 1)
	{
		VulkanApp::_focusWindow = window;
	}
	else
	{
		VulkanApp::_focusWindow = NULL;
	}
}

void GLFWKeyBoard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	using namespace HInput;
	Input::KeyProcess((KeyCode)key, (KeyMod)mods, (Action)action);
}

void GLFWMouseButton(GLFWwindow* window, int button, int action, int mods)
{

}

void GLFWCursorPos(GLFWwindow* window, double xpos, double ypos)
{

}

void GLFWCursorEnter(GLFWwindow* window, int entered)
{

}

void GLFWScroll(GLFWwindow* window, double xoffset, double yoffset)
{

}

void GLFWDrop(GLFWwindow* window, int path_count, const char* paths[])
{
	for (auto& i : VulkanApp::_dropFuns)
	{
		i(path_count, paths);
	}
}

void GLFWJoystick(int jid, int event)
{

}

void GLFWMonitor(GLFWmonitor* monitor, int event)
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
	auto win = CreateNewWindow(64, 64, "MainRenderer", true);
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
	if (_glfwWindows.size() > 0)
	{
		for (int i = 0; i < _glfwWindows.size(); i++)
		{
			if (_glfwWindows[i].renderer)
			{
				_glfwWindows[i].renderer->Release();
				_glfwWindows[i].renderer = NULL;
			}
			if (_glfwWindows[i].window)
			{
				glfwSetWindowShouldClose(_glfwWindows[i].window, true);
			}
		}
		_glfwWindows.clear();
	}
	Shader::DestroyAllShaderCache();
	PipelineManager::ClearPipelineObjects();
	VulkanManager::ReleaseManager();
	glfwTerminate();
}

bool VulkanApp::UpdateForm()
{
	using namespace HInput;
	bool quit = true;
	glfwPollEvents();
	for (int i = 0; i < _glfwWindows.size(); i++)
	{
		if (glfwWindowShouldClose(_glfwWindows[i].window))
		{
			RemoveWindow(_glfwWindows[i]);
			i = i - 1;
			return true;
		}
		else if (_glfwWindows[i].renderer)
		{
			quit = false;
			_glfwWindows[i].renderer->Render();
		}
	}
	if (quit)
	{
		return false;
	}
	else
	{
		Input::ClearInput();
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
	glfwSetWindowSizeCallback(window, GLFWResize);
	glfwSetWindowCloseCallback(window, GLFWClose);
	glfwSetWindowFocusCallback(window,GLFWFocus);
	glfwSetKeyCallback(window,GLFWKeyBoard);
	glfwSetMouseButtonCallback(window,GLFWMouseButton);
	glfwSetCursorPosCallback(window,GLFWCursorPos);
	glfwSetCursorEnterCallback(window,GLFWCursorEnter);
	glfwSetScrollCallback(window,GLFWScroll);
	glfwSetDropCallback(window,GLFWDrop);

	VulkanForm newGLFW = {};
	newGLFW.window = window;
	newGLFW.name = title;
	if (bCreateRenderer)
	{
		newGLFW.renderer = new VulkanRenderer((void*)newGLFW.window , title);
	}
	_glfwWindows.push_back(newGLFW);
	return &_glfwWindows[_glfwWindows.size() - 1];
}

bool VulkanApp::IsWindowFocus(void* windowHandle)
{
	if (_focusWindow != NULL)
	{
		return  _focusWindow == windowHandle;
	}
	return false;
}

void VulkanApp::RemoveWindow(VulkanForm& glfwWindow)
{
	auto window = glfwWindow.window;
	auto it = std::remove_if(_glfwWindows.begin(), _glfwWindows.end(), [window](VulkanForm& glfw) {
		return window == glfw.window;
		});
	if (it != _glfwWindows.end())
	{
		_glfwWindows.erase(it);
	}
}

void VulkanApp::ResizeWindow(VulkanForm& glfwWindow, uint32_t w, uint32_t h)
{
	if (w < 1 || h < 1)
	{
		return;
	}
	if(glfwWindow.window)
		glfwSetWindowSize(glfwWindow.window, (int)w, (int)h);
}

void VulkanApp::SetWindowPos(VulkanForm& glfwWindow, uint32_t x, uint32_t y)
{
	if (glfwWindow.window)
		glfwSetWindowPos(glfwWindow.window, (int)x, (int)y);
}

void* VulkanApp::GetWindowHandle(VulkanForm form)
{
	if (form .window!=NULL)
	{
		#if defined(_WIN32)
		return (void*)glfwGetWin32Window((GLFWwindow*)form.window);
		#endif
	}
	return NULL;
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