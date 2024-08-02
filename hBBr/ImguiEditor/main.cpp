
#include "FormMain.h"

#ifdef _WIN32
#pragma comment(lib , "RendererCore.lib")
#endif

int main(int argc, char* argv[])
{
	//SDL_ShowSimpleMessageBox(0,"","",nullptr);
	//ConsoleDebug::CreateConsole("");
	//Enable custom loop

	VulkanApp::InitVulkanManager(true, true);

	VulkanApp::DeInitVulkanManager();

	return 0;
}
