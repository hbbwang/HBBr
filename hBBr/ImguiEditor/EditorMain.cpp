
#include "GLFWInclude.h"
#include "EditorMain.h"
#include "VulkanRenderer.h"
#include "Pass/ImguiPass.h"
#include "Pass/ImguiPassEditor.h"
#include "Imgui/imgui.h"
#include "RendererConfig.h"

EditorMain::EditorMain()
{
	_mainForm = VulkanApp::GetMainForm();
	_editorGui = _mainForm->renderer->GetEditorGuiPass();
	//
	auto mainEditorWidget = [](struct ImGuiContext* content) {
		ImGui::SetCurrentContext(content);
		ImGuiID mainEditorDock = ImGui::GetID("MainEditorDock");
		ImGui::DockSpaceOverViewport(mainEditorDock);
		
		if (ImGui::Begin("Scene"))
		{

		}
		ImGui::End();

	};
	_editorGui->AddGui(mainEditorWidget);
}
