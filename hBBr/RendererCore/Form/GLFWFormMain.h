#pragma once
#include "Common.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
#include "FileSystem.h"
struct HBBR_API VulkanGLFW
{
	GLFWwindow* window = NULL;
	VulkanRenderer* renderer = NULL;
};

class VulkanApp
{
public:
	/* 初始化Vulkan manager 和 第一个Vulkan渲染窗口
		@bCustomRenderLoop :是否开启自循环
		@bEnableDebug :是否开启Debug layer
		@return:如果bCustomRenderLoop为false,则返回这个vulkan渲染窗口
	*/
	HBBR_API static GLFWwindow* InitVulkanManager(bool bCustomRenderLoop,bool bEnableDebug = false);

	HBBR_API static void DeInitVulkanManager();
	HBBR_API static GLFWwindow* CreateNewWindow(uint32_t w = 512, uint32_t h = 512, const char* title = "Renderer",bool bCreateRenderer = false);
	HBBR_API static std::vector<VulkanGLFW> GetWindows() { return _glfwWindows; }

private:
	static std::vector<VulkanGLFW> _glfwWindows;

};

