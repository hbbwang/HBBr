
#include "SDLInclude.h"
#include "EditorMain.h"
#include "VulkanRenderer.h"
#include "Pass/ImguiPass.h"
#include "Pass/ImguiPassEditor.h"
#include "Imgui/imgui.h"
#include "Imgui/backends/imgui_impl_vulkan.h"
#include "RendererConfig.h"
#include "DescriptorSet.h"

EditorMain::EditorMain()
{
	_mainForm = VulkanApp::GetMainForm();
	_editorGui = _mainForm->swapchain->GetEditorGuiPass();

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
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::Begin(RenderView.c_str()))
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

			ImGui::Image(pass->GetRenderView(), contentRegionSize);

			if (_renderViewSize.width <= 0 || _renderViewSize.height <= 0)
			{
				_renderViewSize = {2,2};
			}
			//调整渲染纹理大小
			_mainForm->swapchain->GetRenderers().begin()->second->SetRenderSize(_renderViewSize);
		}
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	};
	_editorGui->AddGui(mainEditorWidget);
}
