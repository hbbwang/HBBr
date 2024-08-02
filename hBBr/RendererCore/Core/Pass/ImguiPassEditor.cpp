#include "ImguiPassEditor.h"
#include "VulkanRenderer.h"
#include "imgui.h"
#include "SceneTexture.h"
#include "FormMain.h"
/*
	Imgui buffer pass 
*/

#if ENABLE_IMGUI
#include "Imgui/imgui.h"
#include "Imgui/backends/imgui_impl_sdl3.h"
#include "Imgui/backends/imgui_impl_vulkan.h"
#endif

#pragma region ImguiPass 
void ImguiPassEditor::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, -1);
	CreateRenderPass();
	_imguiContent = VulkanManager::GetManager()->InitImgui_SDL(_renderer->GetWindowHandle(), _renderPass, true, true);
	for (auto& i : VulkanApp::GetForms())
	{
		if (i->renderer == _renderer)
		{
			i->imguiContents.push_back(_imguiContent);
		}
	}
	_passName = "Imgui Render Pass";
}

ImguiPassEditor::ImguiPassEditor(VulkanRenderer* renderer)
	:GraphicsPass(nullptr)
{
	_renderer = renderer;
}

ImguiPassEditor::~ImguiPassEditor()
{
	ImGui::SetCurrentContext(_imguiContent);
	VulkanManager::GetManager()->ShutdownImgui();
	if (_imguiContent != nullptr)
		ImGui::DestroyContext(_imguiContent);
}

void ImguiPassEditor::PassReset()
{
	const auto& vkManager = VulkanManager::GetManager();
	glm::mat4 pre_rotate_mat = glm::mat4(1);
	vkManager->ResetImgui_SDL(_renderPass, 0, pre_rotate_mat);
}

void ImguiPassEditor::PassUpdate(std::vector<VkImageView> frameBuffers)
{
	if (ImGui::GetCurrentContext() != _imguiContent)
	{
		ImGui::SetCurrentContext(_imguiContent);
	}
	const auto& vkManager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.1, 0.4, 0.2, 0.2));
	//Update FrameBuffer
	ResetFrameBufferCustom(_renderer->GetRenderSize(), frameBuffers);
	SetViewport(_renderer->GetRenderSize());
	BeginRenderPass({ 0,0,0,0 });
	vkManager->ImguiNewFrame();
	//Begin
	//ImGui::ShowDemoWindow((bool*)1);
	for (auto& i : _gui_extensions)
	{
		i(_imguiContent);
	}
	//End
	vkManager->ImguiEndFrame(cmdBuf);
	EndRenderPass();
}

void ImguiPassEditor::AddGui(std::function<void(struct ImGuiContext*)> fun)
{
	_gui_extensions.push_back(fun);
}

void ImguiPassEditor::PassUpdate()
{
	
}

#pragma endregion ImguiPass 