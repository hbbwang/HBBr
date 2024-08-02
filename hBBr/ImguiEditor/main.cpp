#ifdef _WIN32
#pragma comment(lib , "vulkan-1.lib")
#pragma comment(lib , "RendererCore.lib")
#endif

#include "FormMain.h"
#include "VulkanRenderer.h"
#include "EditorMain.h"

int _bInit = false;

EditorMain* _mainEditor = nullptr;

int main(int argc, char* argv[])
{
	//SDL_ShowSimpleMessageBox(0,"","",nullptr);
	//ConsoleDebug::CreateConsole("");
	//Enable custom loop
	auto mainForm = VulkanApp::InitVulkanManager(false, true);

	while (VulkanApp::UpdateForm())
	{
		if (!_bInit && mainForm->renderer->GetEditorGuiPass())
		{
			_bInit = true;
			_mainEditor = new EditorMain;
		}
		
	}
	VulkanApp::DeInitVulkanManager();
	delete _mainEditor;
	return 0;
}
