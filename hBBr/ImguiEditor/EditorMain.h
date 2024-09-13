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

	std::string MainMenu_File;
	std::string MainMenu_File_NewWorld;
	std::string MainMenu_File_OpenWorld;
	std::string MainMenu_File_SaveWorld;
	std::string MainMenu_Edit;
	std::string MainMenu_Tool;
	std::string SceneOutlineTitle;
	std::string InspectorTitle;
	std::string ContentBrowserTitle;
	std::string RenderViewTitle;
	std::string LoadLevel;
	std::string UnloadLevel;

	std::vector<class Level*> _selectionLevels;

private:

	void GlobalSetting();
	bool& GetLevelActive(class Level* level);
	bool& GetGameObjectActive(class GameObject* obj);
	void BuildMainMenu(ImguiPassEditor* pass);
	void BuildRenderer(ImguiPassEditor* pass);
	void BuildSceneOutline(ImguiPassEditor* pass);
	void BuildInspector(ImguiPassEditor* pass);
	void BuildContentBrowser(ImguiPassEditor* pass);
	void BuildSceneOutlineTreeNode_GameObject(class GameObject* obj, float levelItemXPos, int depth);

};

