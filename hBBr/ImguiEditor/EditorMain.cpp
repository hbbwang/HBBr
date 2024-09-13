
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

	MainMenu_File_Title = GetEditorInternationalizationText("MainWindow", "MainMenu_File_Title").c_str();
	MainMenu_File_NewWorld = GetEditorInternationalizationText("MainWindow", "MainMenu_File_NewWorld").c_str();
	MainMenu_File_OpenWorld = GetEditorInternationalizationText("MainWindow", "MainMenu_File_OpenWorld").c_str();
	MainMenu_File_SaveWorld = GetEditorInternationalizationText("MainWindow", "MainMenu_File_SaveWorld").c_str();
	MainMenu_Edit_Title = GetEditorInternationalizationText("MainWindow", "MainMenu_Edit_Title").c_str();
	MainMenu_Tool_Title = GetEditorInternationalizationText("MainWindow", "MainMenu_Tool_Title").c_str();
	SceneOutlineTitle = GetEditorInternationalizationText("MainWindow", "SceneOutlineTitle").c_str();
	Inspector = GetEditorInternationalizationText("MainWindow", "Inspector").c_str();
	ContentBrowserTitle = GetEditorInternationalizationText("MainWindow", "ContentBrowserTitle").c_str();
	RenderView = GetEditorInternationalizationText("MainWindow", "RenderView").c_str();
	LoadLevel = GetEditorInternationalizationText("SceneOutline", "LoadLevel").c_str();
	UnloadLevel = GetEditorInternationalizationText("SceneOutline", "UnloadLevel").c_str();

	auto mainEditorWidget = [this](ImguiPassEditor* pass) {
		ImGui::SetCurrentContext(pass->_imguiContent);
		ImGuiID mainEditorDock = ImGui::GetID("MainEditorDock");
		ImGui::DockSpaceOverViewport(mainEditorDock);

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu(MainMenu_File_Title.c_str()))
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
			if (ImGui::BeginMenu(MainMenu_Edit_Title.c_str()))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(MainMenu_Tool_Title.c_str()))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

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

				auto clearSelection = [&]() {
					if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
					{
						for (auto& l : world->GetLevels())
						{
							l->_bSelected = false;
							//for (auto& g : l->GetAllGameObjects())
							//{
							//	g->_bSelected = false;
							//}
						}
						_selectionLevels.clear();
					}
					};
				if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
				{
					clearSelection();
				}
				int id = 0;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(FLT_MIN, 4.f));
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 0.f));
				//Tree Node Begin
				{
					for (auto& l : world->GetLevels())
					{
						const auto& allObjects = l->GetAllGameObjects();

						float levelItemXPos = ImGui::GetCursorPosX();
						ImGui::SetCursorPosX(levelItemXPos + 30);

						//Level TreeNode
						l->_bEditorOpen = ImGui::TreeNodeEx(l->GetLevelName().c_str(),
							(ImTextureID)EditorResource::Get()->_icon_levelIcon->descriptorSet,
							treeNodeFlags | (l->_bSelected ? ImGuiTreeNodeFlags_Selected : 0) |
							(l->IsLoaded() ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_CustomArrow));
						/*if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
						{
							ImGui::SetDragDropPayload((l->GetLevelName() + "Drag").c_str(), l->GetLevelName().c_str(), sizeof(const char*));
							ImGui::Text("Dragging %s", l->GetLevelName().c_str());
							ImGui::EndDragDropSource();
						}*/

						if (ImGui::BeginPopupContextItem())
						{
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

						if (ImGui::IsItemClicked())
						{
							if (ImGui::GetIO().KeyCtrl)
							{
								l->_bSelected = true;
							}
							else
							{
								clearSelection();
								l->_bSelected = true;
							}
							_selectionLevels.push_back(l.get());
						}

						//Level CheckBox
						if(l->IsLoaded())
						{
							auto framePadding = ImGui::GetStyle().FramePadding;
							ImGui::SameLine();
							ImGui::PushID(id);
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
									BuildSceneOutlineTreeNode(o.get(), levelItemXPos);
								}
							}
							ImGui::TreePop();
						}
						id++;
					}
				}//Tree Node End
				ImGui::PopStyleVar();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin(Inspector.c_str()))
		{


		}
		ImGui::End();

		if (ImGui::Begin(ContentBrowserTitle.c_str()))
		{


		}
		ImGui::End();

		//主渲染窗口
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		if (ImGui::Begin(RenderView.c_str(), 0))
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

			for (auto& i : _mainRnederer->GetPassManagers())
			{
				i.second->GetImguiPass()->UpdateImguiFocusContentRect(
					HBox2D(
						glm::vec2(ImGui::GetWindowPos().x + contentRegionMin.x, ImGui::GetWindowPos().y + contentRegionMin.y),
						glm::vec2(ImGui::GetWindowSize().x , ImGui::GetWindowSize().y)
					)
				);
			}

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

void EditorMain::GlobalSetting()
{
	auto& style = ImGui::GetStyle();
	style.FrameRounding = 5.0f;
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
	style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 1.0f, 1.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
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
	style.Colors[ImGuiCol_Separator] = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.4f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.65f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
}

bool& EditorMain::GetLevelActive(class Level* level)
{
	return level->_bActive;
}

bool& EditorMain::GetGameObjectActive(GameObject* obj)
{
	return obj->_bActive;
}

void EditorMain::BuildSceneOutlineTreeNode(class GameObject* obj, float levelItemXPos)
{
	ImGuiTreeNodeFlags treeNodeFlags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanFullWidth |
		ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding |
		(obj->_children.size() <= 0 ? ImGuiTreeNodeFlags_CustomArrow : ImGuiTreeNodeFlags_None);
		;
	{
		float objectlItemXPos = ImGui::GetCursorPosX();
		ImGui::SetCursorPosX(objectlItemXPos + 30);
		//Object TreeNode
		obj->_bEditorOpen = ImGui::TreeNodeEx(obj->GetObjectName().c_str(),
			(ImTextureID)EditorResource::Get()->_icon_objectIcon->descriptorSet, 
			treeNodeFlags | (obj->_bSelected ? ImGuiTreeNodeFlags_Selected : 0));
		if (ImGui::IsItemClicked())
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				obj->_bSelected = true;
			}
			else
			{
				obj->_bSelected = true;
			}
		}
		if (obj->_bEditorOpen)
		{
			for (auto& o : obj->GetChildren())
			{
				BuildSceneOutlineTreeNode(o, levelItemXPos);
			}	
			ImGui::TreePop();
		}

		//Object CheckBox
		ImGui::SameLine();
		ImGui::PushID(obj->GetGUID().str().c_str());
		ImGui::SetCursorPosX(levelItemXPos);
		ImGui::Checkbox("", &GetGameObjectActive(obj));
		ImGui::PopID();
	}
}
