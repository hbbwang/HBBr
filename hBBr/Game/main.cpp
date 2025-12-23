#pragma once
#include "Common.h"
#include<iostream>
#include "../Renderer/Core/VulkanApp.h"
#ifdef _WIN32
#pragma comment(lib,"Renderer.lib")
#endif

int main() 
{	
	//Initialize vulkan manager
	VulkanApp::Get()->InitVulkanManager(true);
	//Initialize window with vulkan renderer
	VulkanApp::Get()->CreateVulkanWindow(256, 256, "MainRenderer");
	//Main loop
	while (VulkanApp::Get()->MainLoop())
	{

	}
	//Release vulkan app
	VulkanApp::Get()->Release();
	return 0;
}