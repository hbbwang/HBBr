#ifdef _WIN32
#pragma comment(lib , "vulkan-1.lib")
#pragma comment(lib , "RendererCore.lib")
#endif

#include "FormMain.h"
#include "VulkanRenderer.h"
#include "RendererConfig.h"

int _bInit = false;

int main(int argc, char* argv[])
{
	//SDL_ShowSimpleMessageBox(0,"","",nullptr);
	//ConsoleDebug::CreateConsole("");
	//Enable custom loop
	auto mainForm = VulkanApp::InitVulkanManager(false, true);
	mainForm->closeCallbacks.push_back(
		[](VulkanForm* form) {
			int x, y, w, h;
			VulkanApp::GetWindowPos(form->window, x, y);
			VulkanApp::GetWindowSize(form->window, w, h);
			RenderConfig::GetRendererConfig()["Default"][(form->name + "_WindowPosX").c_str()] = x;
			RenderConfig::GetRendererConfig()["Default"][(form->name + "_WindowPosY").c_str()] = y;
			RenderConfig::GetRendererConfig()["Default"][(form->name + "_WindowWidth").c_str()] = w;
			RenderConfig::GetRendererConfig()["Default"][(form->name + "_WindowHeight").c_str()] = h;
			SaveRendererConfig();
		}
	);
	//³õÊ¼»¯´°¿Ú
	VulkanApp::SetWindowTitle(mainForm->window, GetEditorInternationalizationText("MainWindow", "MainTitle").c_str());
	auto x = GetRendererConfigInt("Default", mainForm->name + "_WindowPosX");
	auto y = GetRendererConfigInt("Default", mainForm->name + "_WindowPosY");
	auto w = GetRendererConfigInt("Default", mainForm->name + "_WindowWidth");
	auto h = GetRendererConfigInt("Default", mainForm->name + "_WindowHeight");
	SDL_DisplayID displayIndex = SDL_GetDisplayForWindow(mainForm->window);
	SDL_Rect screenRect = {};
	if (SDL_GetDisplayBounds(displayIndex, &screenRect) == 0)
	{
		if (w > screenRect.w)
		{
			w = screenRect.w;
		}
		if (h > screenRect.h)
		{
			h = screenRect.h;
		}
		if (x >= screenRect.x + screenRect.w - w / 2)
		{
			x = screenRect.x + screenRect.w - w;
		}
		if (y >= screenRect.y + screenRect.h - h / 2)
		{
			y = screenRect.y + screenRect.h - h;
		}
	}
	VulkanApp::SetWindowPos(mainForm->window, x, y);
	VulkanApp::ResizeWindow(mainForm->window, w, h);
	//
	while (VulkanApp::UpdateForm())
	{
	}
	//
	VulkanApp::DeInitVulkanManager();
	return 0;
}
