
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

		//����Ⱦ����
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0)); // ����͸��������ɫ
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // �Ƴ������ڱ߾�
		if (ImGui::Begin(RenderView.c_str(),0))
		{
			// ��ȡ�����������С��������꣨����ڴ��ڵ����꣩
			ImVec2 contentRegionMin = ImGui::GetWindowContentRegionMin();
			ImVec2 contentRegionMax = ImGui::GetWindowContentRegionMax();

			// ������������Ĵ�С
			ImVec2 contentRegionSize;
			contentRegionSize.x = contentRegionMax.x - contentRegionMin.x;
			contentRegionSize.y = contentRegionMax.y - contentRegionMin.y;

			// ��ȡ��ǰ ImGui ��ʽ
			const ImGuiStyle& style = ImGui::GetStyle();
			// ��ȡ���ڱ߿��С
			float windowBorderSize = style.WindowBorderSize;

			_renderViewSize.width = (uint32_t)std::max(2.0f, contentRegionSize.x);
			_renderViewSize.height = (uint32_t)std::max(2.0f, contentRegionSize.y);
			//_renderViewSize.width += 14;
			//_renderViewSize.height += 15;

			//ImGui::SetCursorPos({ ImGui::GetWindowContentRegionMin().x - x, ImGui::GetWindowContentRegionMin().y - y });
			
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
			//������Ⱦ�����С
			_mainRnederer->SetRenderSize(_renderViewSize);
		}
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	};
	_editorGui->AddGui(mainEditorWidget);
}
