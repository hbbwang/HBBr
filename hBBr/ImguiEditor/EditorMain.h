#pragma once
#include "FormMain.h"
#include <memory>

class EditorMain
{
public:
	EditorMain();

	VulkanForm* _mainForm = nullptr;

	class ImguiPassEditor* _editorGui = nullptr;

	class VulkanRenderer* _mainRnederer = nullptr;

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
	std::string LoadLevel;
	std::string UnloadLevel;

	std::vector<class Level*> _selectionLevels;

private:

	void GlobalSetting();
	bool& GetLevelActive(class Level* level);
	bool& GetGameObjectActive(class GameObject* obj);
	void BuildSceneOutlineTreeNode(class GameObject* obj, float levelItemXPos);

};

