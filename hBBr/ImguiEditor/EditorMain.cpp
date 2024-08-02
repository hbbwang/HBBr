#include "EditorMain.h"
#include "VulkanRenderer.h"
#include "Pass/ImguiPass.h"
#include "Pass/ImguiPassEditor.h"

EditorMain::EditorMain()
{
	_mainForm = VulkanApp::GetMainForm();
	//
	_editorGui = _mainForm->renderer->GetEditorGuiPass();
	//
}
