#pragma once
#include "Common.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
#include "FileSystem.h"
struct VulkanForm
{
	HString name;
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
	HBBR_API static VulkanForm* InitVulkanManager(bool bCustomRenderLoop,bool bEnableDebug = false);
	HBBR_API static void DeInitVulkanManager();
	HBBR_API static VulkanForm* CreateNewWindow(uint32_t w = 512, uint32_t h = 512, const char* title = "Renderer",bool bCreateRenderer = false);
	HBBR_API static std::vector<VulkanForm>& GetWindows() { return _glfwWindows; }
	HBBR_API static void RemoveWindow(VulkanForm& glfwWindow);
	HBBR_API static void ResizeWindow(VulkanForm& glfwWindow , uint32_t w, uint32_t h);
	HBBR_API static void SetWindowPos(VulkanForm& glfwWindow, uint32_t x, uint32_t y);
	HBBR_API static void* GetWindowHandle(VulkanForm& glfwWindow);

private:
	static std::vector<VulkanForm> _glfwWindows;

};

