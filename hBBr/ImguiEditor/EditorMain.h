#pragma once
#include "FormMain.h"
#include <memory>

class EditorMain
{
public:
	EditorMain();

	VulkanForm* _mainForm = nullptr;

	class ImguiPassEditor* _editorGui = nullptr;

	VkExtent2D _renderViewSize;

	int _displayMode = 0;

	std::string MainMenu_File_Title;
	std::string MainMenu_File_NewWorld;
	std::string MainMenu_File_OpenWorld;
	std::string MainMenu_File_SaveWorld;
	std::string MainMenu_Edit_Title;
	std::string MainMenu_Tool_Title;
	std::string SceneOutlineTitle;
	std::string Inspector;
	std::string ContentBrowserTitle;
	std::string RenderView;

};

