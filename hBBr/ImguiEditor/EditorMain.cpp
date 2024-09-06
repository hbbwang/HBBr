
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

	auto mainEditorWidget = [this](ImguiPassEditor* pass) {
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
					ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
				auto world = _mainRnederer->GetWorld().lock();

				auto clearSelection = [&]() {
					if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
					{
						for (auto& l : world->GetLevels())
						{
							l->_bSelected = false;
							for (auto& g : l->GetAllGameObjects())
							{
								g->_bSelected = false;
							}
						}
					}
				};
				if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
				{
					clearSelection();
				}
				int id = 0;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(FLT_MIN, 4.f));
				for (auto& l : world->GetLevels())
				{
					static bool checkBox = true;
					float levelItemXPos = ImGui::GetCursorPosX();
					ImGui::SetCursorPosX(levelItemXPos + 30);
					//Level Icon
					//ImGui::Image(
					//	(ImTextureID)EditorResource::Get()->_icon_levelIcon->descriptorSet,
					//	ImVec2(26, 26));
					//ImGui::SameLine();
					//Level TreeNode
					bool isLevelOpen = ImGui::TreeNodeEx(l->GetLevelName().c_str(), (ImTextureID)EditorResource::Get()->_icon_levelIcon->descriptorSet, treeNodeFlags | (l->_bSelected ? ImGuiTreeNodeFlags_Selected : 0));
					/*if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) 
					{
						ImGui::SetDragDropPayload((l->GetLevelName() + "Drag").c_str(), l->GetLevelName().c_str(), sizeof(const char*));
						ImGui::Text("Dragging %s", l->GetLevelName().c_str());
						ImGui::EndDragDropSource();
					}*/
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
					}
					//Level CheckBox
					ImGui::SameLine();
					ImGui::PushID(id);
					ImGui::SetCursorPosX(levelItemXPos);
					ImGui::Checkbox("", &checkBox);
					ImGui::PopID();
					if (isLevelOpen)
					{
						for (auto& g : l->GetAllGameObjects())
						{
							float objectlItemXPos = ImGui::GetCursorPosX();
							ImGui::SetCursorPosX(objectlItemXPos + 30);
							//Object Icon
							//ImGui::Image(
							//	(ImTextureID)EditorResource::Get()->_icon_objectIcon->descriptorSet,
							//	ImVec2(26, 26));
							//ImGui::SameLine();
							//Object TreeNode
							bool isObjectOpen = ImGui::TreeNodeEx(g->GetObjectName().c_str(), (ImTextureID)EditorResource::Get()->_icon_objectIcon->descriptorSet,treeNodeFlags | (g->_bSelected ? ImGuiTreeNodeFlags_Selected : 0));
							if (ImGui::IsItemClicked())
							{
								if (ImGui::GetIO().KeyCtrl)
								{
									g->_bSelected = true;
								}
								else
								{
									clearSelection();
									g->_bSelected = true;
								}
							}
							if (isObjectOpen)
							{
								ImGui::TreePop();
							}
							//Object CheckBox
							ImGui::SameLine();
							ImGui::PushID(id);
							ImGui::SetCursorPosX(levelItemXPos);
							ImGui::Checkbox("", &checkBox);
							ImGui::PopID();
							id++;
						}
						ImGui::TreePop();
					}
					id++;
				}
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
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0)); // 设置透明背景颜色
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // 移除窗口内边距
		if (ImGui::Begin(RenderView.c_str(),0))
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
					{
						(int)(ImGui::GetWindowPos().x + contentRegionMin.x),
						(int)(ImGui::GetWindowPos().y + contentRegionMin.y),
						(int)ImGui::GetWindowSize().x,
						(int)ImGui::GetWindowSize().y
					}
				);
			}

			ImGui::Image(pass->GetRenderView(), contentRegionSize);

			if (_renderViewSize.width <= 0 || _renderViewSize.height <= 0)
			{
				_renderViewSize = {2,2};
			}
			//调整渲染纹理大小
			_mainRnederer->SetRenderSize(_renderViewSize);
		}
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();


		ImGui::ShowDemoWindow((bool*)1);

	};
	_editorGui->AddGui(mainEditorWidget);
}
