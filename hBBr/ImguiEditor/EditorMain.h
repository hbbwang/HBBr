#pragma once
#include "FormMain.h"
#include <memory>
#include "HGuid.h"
#include <map>

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

	struct SceneOutlineTreeNodeData 
	{
		class GameObject* object;
	};
	std::vector<SceneOutlineTreeNodeData> _sceneOutlineTreeNodeData;

	struct BrowserContentNode 
	{
		bool bSelected;
		std::string name;
		HGUID parentGuid;
		std::vector<BrowserContentNode*> children;
		class AssetInfoBase* assetInfo;
		int attachmentDepth = 0;
		bool bEditorOpen = false;
	};
	std::map<HGUID, BrowserContentNode> _cb_folders;
	std::vector<BrowserContentNode> _cb_files;

private:

	void GlobalSetting();
	void SceneOutlineClearSelection(class World*world);
	bool& GetLevelActive(class Level* level);
	bool& GetGameObjectActive(class GameObject* obj);
	void BuildMainMenu(ImguiPassEditor* pass);
	void BuildRenderer(ImguiPassEditor* pass,float windowX,float windowY);
	void BuildSceneOutline(ImguiPassEditor* pass);
	void BuildInspector(ImguiPassEditor* pass);
	void BuildContentBrowser(ImguiPassEditor* pass);
	void BuildContentBrowser_Folders(BrowserContentNode& node, float levelItemXPos, int depth);
	void BuildContentBrowser_Files(BrowserContentNode& node);
	void BuildSceneOutlineTreeNode_GameObject(std::shared_ptr<class GameObject> obj, float levelItemXPos, int depth, char* searchInput, int searchInputLength);

};

