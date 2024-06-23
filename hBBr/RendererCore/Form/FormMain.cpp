#include "FormMain.h"
#include "VulkanRenderer.h"
#include <map>

VulkanForm::~VulkanForm()
{
	if (renderer)
	{
		renderer->Release();
		renderer = nullptr;
	}
	if (window)
	{
		SDL_DestroyWindow(window);
	}
}
