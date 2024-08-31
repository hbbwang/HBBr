#include "FormMain.h"
#include "VulkanRenderer.h"
#include <map>

VulkanForm::~VulkanForm()
{
	if (swapchain)
	{
		swapchain->Release();
		swapchain = nullptr;
	}
	if (window)
	{
		SDL_DestroyWindow(window);
	}
}
