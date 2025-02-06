
#include "SDLInclude.h"
#include "EditorMain.h"
#include "VulkanRenderer.h"
#include "Pass/PassManager.h"
#include "Pass/ImguiPass.h"
#include "Pass/ImguiPassEditor.h"
#include "Imgui/imgui.h"
#include "Imgui/backends/imgui_impl_vulkan.h"
#include "RendererConfig.h"
#include "DescriptorSet.h"
#include "World.h"
#include "EditorResource.h"
#include "GameObject.h"

EditorMain::EditorMain()
{
	_mainForm = VulkanApp::GetMainForm();
	_editorGui = _mainForm->swapchain->GetEditorGuiPass();
	_mainRnederer = _mainForm->swapchain->GetRenderers().begin()->second;

	//MainMenu text
	MainMenu_File = GetEditorInternationalizationText("MainMenu", "MainMenu_File").c_str();
	MainMenu_File_NewWorld = GetEditorInternationalizationText("MainMenu", "MainMenu_File_NewWorld").c_str();
	MainMenu_File_OpenWorld = GetEditorInternationalizationText("MainMenu", "MainMenu_File_OpenWorld").c_str();
	MainMenu_File_SaveWorld = GetEditorInternationalizationText("MainMenu", "MainMenu_File_SaveWorld").c_str();
	MainMenu_Edit = GetEditorInternationalizationText("MainMenu", "MainMenu_Edit").c_str();
	MainMenu_Tool = GetEditorInternationalizationText("MainMenu", "MainMenu_Tool").c_str();

	//Widget text
	SceneOutlineTitle = GetEditorInternationalizationText("MainWindow", "SceneOutlineTitle").c_str();
	InspectorTitle = GetEditorInternationalizationText("MainWindow", "InspectorTitle").c_str();
	ContentBrowserTitle = GetEditorInternationalizationText("MainWindow", "ContentBrowserTitle").c_str();
	RenderViewTitle = GetEditorInternationalizationText("MainWindow", "RenderViewTitle").c_str();

	//Context text
	LoadLevel = GetEditorInternationalizationText("SceneOutline", "LoadLevel").c_str();
	UnloadLevel = GetEditorInternationalizationText("SceneOutline", "UnloadLevel").c_str();

	auto mainEditorWidget = [this](ImguiPassEditor* pass) {
		ImGui::SetCurrentContext(pass->_imguiContent);
		ImGuiID mainEditorDock = ImGui::GetID("MainEditorDock");
		ImGui::DockSpaceOverViewport(mainEditorDock);

		BuildMainMenu(pass);

		BuildSceneOutline(pass);

		BuildInspector(pass);

		BuildContentBrowser(pass);

		BuildRenderer(pass);

		//ImGui::ShowDemoWindow((bool*)1);
		ImGui::ShowStyleEditor();

		};
	_editorGui->AddGui(mainEditorWidget);

	auto editorWidgetBegin = [this](ImguiPassEditor* pass) {
		ImGui::SetCurrentContext(pass->_imguiContent);

		static bool bOK = false;

		//GlobalSetting
		if (!bOK)
		{
			bOK = true;
			GlobalSetting();
		}
	};
	_editorGui->AddGui(editorWidgetBegin);
}

bool& EditorMain::GetLevelActive(class Level* level)
{
	return level->_bActive;
}

bool& EditorMain::GetGameObjectActive(GameObject* obj)
{
	return obj->_bActive;
}

void EditorMain::BuildMainMenu(ImguiPassEditor* pass)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu(MainMenu_File.c_str()))
		{
			if (ImGui::MenuItem(MainMenu_File_NewWorld.c_str(), nullptr))
			{

			}
			if (ImGui::MenuItem(MainMenu_File_OpenWorld.c_str(), nullptr))
			{

			}
			if (ImGui::MenuItem(MainMenu_File_SaveWorld.c_str(), nullptr))
			{

			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(MainMenu_Edit.c_str()))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(MainMenu_Tool.c_str()))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void EditorMain::BuildRenderer(ImguiPassEditor* pass)
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin(RenderViewTitle.c_str(), 0))
	{
		// 获取内容区域的最小和最大坐标（相对于窗口的坐标）
		ImVec2 contentRegionMin = ImGui::GetWindowContentRegionMin();
		ImVec2 contentRegionMax = ImGui::GetWindowContentRegionMax();

		// 计算内容区域的大小
		ImVec2 contentRegionSize;
		contentRegionSize.x = contentRegionMax.x - contentRegionMin.x;
		contentRegionSize.y = contentRegionMax.y - contentRegionMin.y;

		// 获取当前 ImGui 样式
		const ImGuiStyle& style = ImGui::GetStyle();
		// 获取窗口边框大小
		float windowBorderSize = style.WindowBorderSize;

		_renderViewSize.width = (uint32_t)std::max(2.0f, contentRegionSize.x);
		_renderViewSize.height = (uint32_t)std::max(2.0f, contentRegionSize.y);
		//_renderViewSize.width += 14;
		//_renderViewSize.height += 15;

		HBox2D rendererRegion = HBox2D(
			glm::vec2(ImGui::GetWindowPos().x + contentRegionMin.x, ImGui::GetWindowPos().y + contentRegionMin.y),
			glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y)
		);

		for (auto& i : _mainRnederer->GetPassManagers())
		{
			i.second->GetImguiPass()->UpdateImguiFocusContentRect(rendererRegion);
		}

		_mainRnederer->EnableRendererRegion(true);
		rendererRegion._max = glm::vec2(rendererRegion._min.x + ImGui::GetWindowSize().x, rendererRegion._min.y + ImGui::GetWindowSize().y);
		_mainRnederer->SetRendererRegion(rendererRegion);

		ImGui::Image(pass->GetRenderView(), contentRegionSize);

		if (_renderViewSize.width <= 0 || _renderViewSize.height <= 0)
		{
			_renderViewSize = { 2,2 };
		}
		//调整渲染纹理大小
		_mainRnederer->SetRenderSize(_renderViewSize);
	}
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

void EditorMain::SceneOutlineClearSelection(class World* world)
{
	for (auto& l : world->GetLevels())
	{
		l->_bSelected = false;
		for (auto& g : l->GetAllGameObjects())
			g->_bSelected = false;
	}
	_selectionLevels.clear();
	_sceneOutlineTreeNodeData.clear();
}

void EditorMain::BuildSceneOutline(ImguiPassEditor* pass)
{
	if (ImGui::Begin(SceneOutlineTitle.c_str()))
	{
		if (!_mainRnederer->GetWorld().expired())
		{
			ImGuiTreeNodeFlags treeNodeFlags =
				ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_OpenOnDoubleClick |
				ImGuiTreeNodeFlags_SpanFullWidth |
				ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding
				;
			auto world = _mainRnederer->GetWorld().lock();

			static char searchInput[1024];
			static int length = 0;
			struct Funcs
			{
				static int SearchCallback(ImGuiInputTextCallbackData* data)
				{
					length = data->BufTextLen;
					VulkanRenderer* mainRenderer = (VulkanRenderer*)data->UserData;
					if (!mainRenderer->GetWorld().expired())
					{
						auto world = mainRenderer->GetWorld().lock();
						for (auto& l : world->GetLevels())
						{
							for (auto& g : l->GetAllGameObjects())
							{
								g->_bSceneOutlineVisible = true;
								g->_bSceneOutlineSearchResult = false;
								if (length > 0)
								{
									auto bContains = StringTool::Contains(g->GetObjectName(), data->Buf, false);
									if (!bContains)
									{
										g->_bSceneOutlineVisible = false;
									}
									else
									{
										g->_bSceneOutlineSearchResult = true;
									}
								}
							}
							if (length > 0)
							{
								for (auto& g : l->GetAllGameObjects())
								{
									// 从最后一层级开始往回找
									if (g->GetChildrenNum() <= 0 && g->_bSceneOutlineVisible)
									{
										auto parent = g->GetParent();
										while (parent != nullptr)
										{
											parent->_bSceneOutlineVisible = true;
											parent = parent->GetParent();
										}
									}
								}
							}
						}
					}
					return 0;
				}
			};

			//Search input
			{
				ImGui::Image((ImTextureID)EditorResource::Get()->_icon_search->descriptorSet, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::InputText("##SceneOutlineSearch", searchInput, 1023, ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackCharFilter, Funcs::SearchCallback, _mainRnederer);
				ImGui::PopItemWidth();
				ImGui::Separator();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(FLT_MIN, 4.f));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0.f));
			//Tree Node Begin
			{
				for (auto& l : world->GetLevels())
				{
					const auto& allObjects = l->GetAllGameObjects();

					float levelItemXPos = ImGui::GetCursorPosX();
					ImGui::SetCursorPosX(levelItemXPos + ImGui::GetFrameHeight());

					//Level TreeNode
					l->_bEditorOpen = ImGui::TreeNodeEx(l->GetLevelName().c_str(),
						(ImTextureID)EditorResource::Get()->_icon_levelIcon->descriptorSet,
						treeNodeFlags | (l->_bSelected ? ImGuiTreeNodeFlags_Selected : 0) |
						(l->IsLoaded() ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_CustomArrow));

					auto changedSelection = [&]() {
						if (ImGui::GetIO().KeyCtrl)
						{
							l->_bSelected = !l->_bSelected;
						}
						else
						{
							if (!l->_bSelected)
							{
								SceneOutlineClearSelection(world.get());
								l->_bSelected = true;
							}
						}
						_selectionLevels.push_back(l.get());
					};

					if (ImGui::BeginPopupContextItem())
					{
						changedSelection();
						if (ImGui::MenuItem(LoadLevel.c_str()))
						{
							for (auto& l : _selectionLevels)
							{
								l->Load();
							}
						}
						if (ImGui::MenuItem(UnloadLevel.c_str()))
						{
							for (auto& l : _selectionLevels)
							{
								l->UnLoad();
							}
						}
						ImGui::EndPopup();
					}

					if ((ImGui::IsItemClicked(0)))
					{
						changedSelection();
					}	

					//Level CheckBox
					if (l->IsLoaded())
					{
						auto framePadding = ImGui::GetStyle().FramePadding;
						ImGui::SameLine();
						ImGui::PushID(l->_guid.str().c_str());
						ImGui::SetCursorPosX(levelItemXPos);
						ImGui::Checkbox("", &GetLevelActive(l.get()));
						ImGui::PopID();
					}

					if (l->_bEditorOpen)
					{
						if (l->IsLoaded())
						{
							for (auto& o : allObjects)
							{
								BuildSceneOutlineTreeNode_GameObject(o, levelItemXPos, 0, searchInput, length);
							}
						}
						ImGui::TreePop();
					}
					//ImGui::Separator();
				}
			}//Tree Node End

			if ((ImGui::IsMouseClicked(0)) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && !ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyShift)
			{
				SceneOutlineClearSelection(world.get());
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

void EditorMain::BuildSceneOutlineTreeNode_GameObject(std::shared_ptr<class GameObject> obj, float levelItemXPos, int depth, char* searchInput, int searchInputLength)
{
	ImGuiTreeNodeFlags treeNodeFlags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanFullWidth |
		ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding 
		| (obj->_children.size() <= 0 ? ImGuiTreeNodeFlags_CustomArrow : ImGuiTreeNodeFlags_None)
		;
	{
		if (obj->_attachmentDepth == depth && obj->_bSceneOutlineVisible)
		{
			float objectlItemXPos = ImGui::GetCursorPosX();
			ImGui::SetCursorPosX(objectlItemXPos + ImGui::GetFontSize());

			//Object TreeNode
			obj->_bEditorOpen = ImGui::TreeNodeEx(obj->GetObjectName().c_str(),
				(ImTextureID)EditorResource::Get()->_icon_objectIcon->descriptorSet,
				treeNodeFlags | (obj->_bSelected ? ImGuiTreeNodeFlags_Selected : 0));

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				ImGui::SetDragDropPayload("SceneOutline_GameObject", &_sceneOutlineTreeNodeData, sizeof(_sceneOutlineTreeNodeData));
				ImGui::Text("---Dragging---");
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget()) 
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneOutline_GameObject")) 
				{
					std::vector<SceneOutlineTreeNodeData>* receivedNodes = static_cast<std::vector<SceneOutlineTreeNodeData>*>(payload->Data);
					if (receivedNodes)
					{
						for (int i = 0; i < receivedNodes->size(); i++)
						{
							GameObject* node = receivedNodes->at(i).object;
							if (node != obj.get())
							{
								node->SetParent(obj.get());
							}
						}
						_sceneOutlineTreeNodeData.clear();
					}
				}
				ImGui::EndDragDropTarget();
			}

			//松开左键
			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				if (ImGui::GetIO().KeyCtrl)
				{
					obj->_bSelected = !obj->_bSelected;
					if (obj->_bSelected == false)
					{
						_sceneOutlineTreeNodeData.erase(std::remove_if(
							_sceneOutlineTreeNodeData.begin(), _sceneOutlineTreeNodeData.end(),
							[&](SceneOutlineTreeNodeData& data) {
								return data.object == obj.get();
							}));
					}

				}
				else
				{
					SceneOutlineClearSelection(obj->GetWorld());
					obj->_bSelected = true;
				}

				if(obj->_bSelected)
				{
					SceneOutlineTreeNodeData newData;
					newData.object = obj.get();
					_sceneOutlineTreeNodeData.push_back(newData);
				}
			}

			if (obj->_bSceneOutlineSearchResult && !obj->_bSelected)
			{
				ImVec2 min = ImGui::GetItemRectMin();
				ImVec2 max = ImGui::GetItemRectMax();
				ImGui::GetForegroundDrawList()->AddRectFilled(
					min,
					max,
					ImGui::GetColorU32(ImGuiCol_TreeNodeItemSearchResult));
			}

			//Object CheckBox
			{
				ImGui::SameLine();
				ImGui::SetCursorPosX(levelItemXPos);
				ImGui::PushID(obj->GetGUID().str().c_str());
				ImGui::Checkbox("", &GetGameObjectActive(obj.get()));
				ImGui::PopID();
			}

			if (obj->_bEditorOpen)
			{
				for (auto& o : obj->GetChildren())
				{
					BuildSceneOutlineTreeNode_GameObject(o->GetSelfWeekPtr().lock(), levelItemXPos, depth + 1, searchInput, searchInputLength);
				}
				ImGui::TreePop();
			}

		}
	}
}

void EditorMain::BuildInspector(ImguiPassEditor* pass)
{
	if (ImGui::Begin(InspectorTitle.c_str()))
	{


	}
	ImGui::End();
}

void DrawSplitter(bool split_vertically, float thickness, float* size0, float* size1, float min_size0, float min_size1)
{
	ImVec2 backup_pos = ImGui::GetCursorPos();
	if (split_vertically)
		ImGui::SetCursorPosY(backup_pos.y + *size0);
	else
		ImGui::SetCursorPosX(backup_pos.x + *size0);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));          // We don't draw while active/pressed because as we move the panes the splitter button will be 1 frame late
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.10f));
	ImGui::Button("##Splitter", ImVec2(!split_vertically ? thickness : -1.0f, split_vertically ? thickness : -1.0f));
	ImGui::PopStyleColor(3);

	ImGui::SetItemAllowOverlap(); // This is to allow having other buttons OVER our splitter. 

	if (ImGui::IsItemActive())
	{
		float mouse_delta = split_vertically ? ImGui::GetIO().MouseDelta.y : ImGui::GetIO().MouseDelta.x;

		// Minimum pane size
		if (mouse_delta < min_size0 - *size0)
			mouse_delta = min_size0 - *size0;
		if (mouse_delta > *size1 - min_size1)
			mouse_delta = *size1 - min_size1;

		// Apply resize
		*size0 += mouse_delta;
		*size1 -= mouse_delta;
	}
	ImGui::SetCursorPos(backup_pos);
}

void EditorMain::BuildContentBrowser(ImguiPassEditor* pass)
{
	static float size1 = 100, size2 = 200;
	if (ImGui::Begin(ContentBrowserTitle.c_str()))
	{
		static float w = 200.0f;
		float h = ImGui::GetContentRegionAvail().y;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		if (ImGui::BeginChild("Folders", ImVec2(w, h), true))
		{
			//auto& folders = ContentManager::Get()->GetVirtualFolders();
			//BrowserContentNode contentNode;
			//contentNode.name = "Content";
			//contentNode.attachmentDepth = 0;
			//contentNode.bEditorOpen = true;
			//for (auto& f : folders)
			//{
			//	BrowserContentNode newNode;
			//	auto name = f.second.FolderName;
			//	newNode.name = name.c_str();
			//	f.second.
			//}
			//BuildContentBrowser_Folders(pass);
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::InvisibleButton("VSplitter", ImVec2(6.0f, h));
		if (ImGui::IsItemActive())
			w += ImGui::GetIO().MouseDelta.x;
		ImGui::SameLine();
		if (ImGui::BeginChild("Files", ImVec2(0, h), true))
		{
			//BuildContentBrowser_Files(pass);
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

void EditorMain::BuildContentBrowser_Folders(BrowserContentNode& node, float levelItemXPos, int depth)
{
	//ImGuiTreeNodeFlags treeNodeFlags =
	//	ImGuiTreeNodeFlags_OpenOnArrow |
	//	ImGuiTreeNodeFlags_OpenOnDoubleClick |
	//	ImGuiTreeNodeFlags_SpanFullWidth |
	//	ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding
	//	| (node.children.size() <= 0 ? ImGuiTreeNodeFlags_CustomArrow : ImGuiTreeNodeFlags_None)
	//	;
	//{
	//	if (node.attachmentDepth == depth)
	//	{
	//		float objectlItemXPos = ImGui::GetCursorPosX();
	//		ImGui::SetCursorPosX(objectlItemXPos + ImGui::GetFontSize());

	//		//Object TreeNode
	//		node.bEditorOpen = ImGui::TreeNodeEx(node.name.c_str(),
	//			(ImTextureID)EditorResource::Get()->_icon_objectIcon->descriptorSet,
	//			treeNodeFlags | (node.bSelected ? ImGuiTreeNodeFlags_Selected : 0));

	//		//if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	//		//{
	//		//	ImGui::SetDragDropPayload("SceneOutline_GameObject", &_sceneOutlineTreeNodeData, sizeof(_sceneOutlineTreeNodeData));
	//		//	ImGui::Text("---Dragging---");
	//		//	ImGui::EndDragDropSource();
	//		//}

	//		//if (ImGui::BeginDragDropTarget())
	//		//{
	//		//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneOutline_GameObject"))
	//		//	{
	//		//		std::vector<SceneOutlineTreeNodeData>* receivedNodes = static_cast<std::vector<SceneOutlineTreeNodeData>*>(payload->Data);
	//		//		if (receivedNodes)
	//		//		{
	//		//			for (int i = 0; i < receivedNodes->size(); i++)
	//		//			{
	//		//				GameObject* node = receivedNodes->at(i).object;
	//		//				if (node != obj.get())
	//		//				{
	//		//					node->SetParent(obj.get());
	//		//				}
	//		//			}
	//		//			_sceneOutlineTreeNodeData.clear();
	//		//		}
	//		//	}
	//		//	ImGui::EndDragDropTarget();
	//		//}

	//		//松开左键
	//		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	//		{
	//			if (ImGui::GetIO().KeyCtrl)
	//			{
	//				node.bSelected = !node.bSelected;
	//				if (node.bSelected == false)
	//				{
	//					//_sceneOutlineTreeNodeData.erase(std::remove_if(
	//					//	_sceneOutlineTreeNodeData.begin(), _sceneOutlineTreeNodeData.end(),
	//					//	[&](SceneOutlineTreeNodeData& data) {
	//					//		return data.object == obj.get();
	//					//	}));
	//				}

	//			}
	//			else
	//			{
	//				ClearSelection(obj->GetWorld());
	//				obj->_bSelected = true;
	//			}

	//			if (obj->_bSelected)
	//			{
	//				SceneOutlineTreeNodeData newData;
	//				newData.object = obj.get();
	//				_sceneOutlineTreeNodeData.push_back(newData);
	//			}
	//		}

	//		if (obj->_bSceneOutlineSearchResult && !obj->_bSelected)
	//		{
	//			ImVec2 min = ImGui::GetItemRectMin();
	//			ImVec2 max = ImGui::GetItemRectMax();
	//			ImGui::GetForegroundDrawList()->AddRectFilled(
	//				min,
	//				max,
	//				ImGui::GetColorU32(ImGuiCol_TreeNodeItemSearchResult));
	//		}

	//		//Object CheckBox
	//		{
	//			ImGui::SameLine();
	//			ImGui::SetCursorPosX(levelItemXPos);
	//			ImGui::PushID(obj->GetGUID().str().c_str());
	//			ImGui::Checkbox("", &GetGameObjectActive(obj.get()));
	//			ImGui::PopID();
	//		}

	//		if (obj->_bEditorOpen)
	//		{
	//			for (auto& o : obj->GetChildren())
	//			{
	//				BuildSceneOutlineTreeNode_GameObject(o->GetSelfWeekPtr().lock(), levelItemXPos, depth + 1, searchInput, searchInputLength);
	//			}
	//			ImGui::TreePop();
	//		}

	//	}
	//}
}

void EditorMain::BuildContentBrowser_Files(BrowserContentNode& node)
{

}

void EditorMain::GlobalSetting()
{
	auto& style = ImGui::GetStyle();
	style.FrameRounding = 4.0f; //圆角
	style.CircleTessellationMaxError = 2.0f;
	style.FrameBorderSize = 1.0f;
	style.WindowMenuButtonPosition = ImGuiDir::ImGuiDir_Right;
	style.WindowPadding = ImVec2(4.0f, 4.0f);
	style.ScrollbarSize = 17.5f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 10.0f;
	style.TabBarOverlineSize = 0.0f;
	style.WindowTitleHeight = 12.0f;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784f, 0.0784f, 0.0784f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 1.0f, 1.0f, 0.35f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
	style.Colors[ImGuiCol_TreeNodeItemSearchResult] = ImVec4(0.2f, 0.3f, 0.4f, 0.3f);
	style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 0.65f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2f, 0.28f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.156f, 0.156f, 0.156f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.125f, 0.16f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1, 1, 1, 0.5);
	style.Colors[ImGuiCol_Tab] = ImVec4(1.0f, 1.0f, 1.0f, 0.15f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
	style.Colors[ImGuiCol_TabSelected] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
	style.Colors[ImGuiCol_TabDimmedSelected] = ImVec4(1.0f, 1.0f, 1.0f, 0.0784f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.35f, 0.45f, 0.55f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.4f, 0.45f, 0.75f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.3f, 0.4f, 0.5f, 0.65f);
	style.Colors[ImGuiCol_Separator] = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.45f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.55f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.4f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.65f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
}
