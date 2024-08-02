#ifdef _WIN32
#pragma comment(lib , "vulkan-1.lib")
#pragma comment(lib , "RendererCore.lib")
#endif

#include "FormMain.h"
#include "VulkanRenderer.h"
#include "EditorMain.h"

int _bInit = false;

int main(int argc, char* argv[])
{
	//SDL_ShowSimpleMessageBox(0,"","",nullptr);
	//ConsoleDebug::CreateConsole("");
	//Enable custom loop

	auto mainForm = VulkanApp::InitVulkanManager(false, true);

	EditorMain mainEditor;

	//≤Â»ÎImguiEditorPass
	while (VulkanApp::UpdateForm())
	{
		if (!_bInit)
		{
			_bInit = true;

		}
		
	}

	VulkanApp::DeInitVulkanManager();

	return 0;
}
