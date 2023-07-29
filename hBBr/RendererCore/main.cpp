#include "GLFWFormMain.h"
#include "Shader.h"

std::vector<VulkanGLFW> VulkanApp::_glfwWindows;

void GLFWResize(GLFWwindow* window, int width, int height)
{
	auto it = std::find_if(VulkanApp::GetWindows().begin(),VulkanApp::GetWindows().end(), [window](VulkanGLFW& glfw)
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
	auto it = std::find_if(VulkanApp::GetWindows().begin(), VulkanApp::GetWindows().end(), [window](VulkanGLFW& glfw)
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

}

void GLFWKeyBoard(GLFWwindow* window, int key, int scancode, int action, int mods)
{

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

}

void GLFWJoystick(int jid, int event)
{

}

void GLFWMonitor(GLFWmonitor* monitor, int event)
{

}

VulkanGLFW* VulkanApp::InitVulkanManager(bool bCustomRenderLoop , bool bEnableDebug)
{
	//must be successful.
	if (!glfwInit())
	{
		MessageOut("Init glfw failed.", true, true, "255,0,0");
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	VulkanManager::InitManager(bEnableDebug);
	Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());

	//Create Main Window
	auto win = CreateNewWindow(64, 64, "MainRenderer", true);

	if (bCustomRenderLoop)
	{
		while (true)
		{
			bool quit = true;
			glfwPollEvents();
			for (int i = 0; i < _glfwWindows.size(); i++)
			{
				if (glfwWindowShouldClose(_glfwWindows[i].window))
				{
					RemoveGLFWWindow(_glfwWindows[i]);
					i = i - 1;
					continue;
				}
				else if (_glfwWindows[i].renderer)
				{
					quit = false;
					_glfwWindows[i].renderer->Render();
				}
			}
			if (quit)
			{
				break;
			}
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
	VulkanManager::ReleaseManager();
	glfwTerminate();
}

VulkanGLFW* VulkanApp::CreateNewWindow(uint32_t w, uint32_t h , const char* title, bool bCreateRenderer)
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

	VulkanGLFW newGLFW = {};
	newGLFW.window = window;
	newGLFW.name = title;
	if (bCreateRenderer)
	{
		newGLFW.renderer = new VulkanRenderer((void*)newGLFW.window , title);
	}
	_glfwWindows.push_back(newGLFW);
	return &_glfwWindows[_glfwWindows.size() - 1];
}

void VulkanApp::RemoveGLFWWindow(VulkanGLFW& glfwWindow)
{
	auto window = glfwWindow.window;
	auto it = std::remove_if(_glfwWindows.begin(), _glfwWindows.end(), [window](VulkanGLFW& glfw) {
		return window == glfw.window;
		});
	if (it != _glfwWindows.end())
	{
		_glfwWindows.erase(it);
	}
}

void VulkanApp::ResizeGLFWWindow(VulkanGLFW& glfwWindow, uint32_t w, uint32_t h)
{
	if (w < 1 || h < 1)
	{
		return;
	}
	if(glfwWindow.window)
		glfwSetWindowSize(glfwWindow.window, (int)w, (int)h);
}

void VulkanApp::SetGLFWWindowPos(VulkanGLFW& glfwWindow, uint32_t x, uint32_t y)
{
	if (glfwWindow.window)
		glfwSetWindowPos(glfwWindow.window, (int)x, (int)y);
}

void VulkanApp::SetSimpleGLFWWindow(VulkanGLFW& glfwWindow)
{
	if (glfwWindow.window)
	{
		// 窗体装饰（GLFW_FALSE：表示去掉边框，标题栏，系统按钮等）
		glfwSetWindowAttrib(glfwWindow.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowAttrib(glfwWindow.window, GLFW_NO_WINDOW_CONTEXT, GLFW_TRUE);
		#if defined(_WIN32)
		SetWindowLong(glfwGetWin32Window(glfwWindow.window), GWL_STYLE, WS_CHILD | WS_VISIBLE);
		#endif
	}
}

void* VulkanApp::GetHandle(VulkanGLFW& glfwWindow)
{
	if (glfwWindow.window)
	{
		#if defined(_WIN32)
		return (void*)glfwGetWin32Window(glfwWindow.window);
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