#include "ImguiPass.h"
#include "VulkanRenderer.h"
#include "imgui.h"
#include "SceneTexture.h"
/*
	Imgui buffer pass 
*/
#pragma region ImguiScreenPass 
void ImguiScreenPass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, -1);
	CreateRenderPass();
	VulkanManager::GetManager()->InitImgui_SDL(_renderer->GetWindowHandle(), _renderPass);
	_passName = "Imgui Render Pass";
}

ImguiScreenPass::~ImguiScreenPass()
{
	VulkanManager::GetManager()->ShutdownImgui();
}

void ImguiScreenPass::PassReset()
{
	const auto manager = VulkanManager::GetManager();
	glm::mat4 pre_rotate_mat = glm::mat4(1);
	manager->ResetImgui_SDL(_renderPass, 0, pre_rotate_mat);
}

void ImguiScreenPass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.1, 0.4, 0.2, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetRenderSize(), {GetSceneTexture(SceneTextureDesc::FinalColor)->GetTextureView()});
	SetViewport(_currentFrameBufferSize);
	BeginRenderPass({ 0,0,0,0 });
	manager->ImguiNewFrame();
	//Begin
	//ImGui::ShowDemoWindow((bool*)1);
	ShowPerformance();
	//End
	manager->ImguiEndFrame(cmdBuf);
	EndRenderPass();
}

void ImguiScreenPass::ShowPerformance()
{
	if (ImGui::Begin("Performance", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs))
	{
		double frameRate = VulkanApp::GetFrameRate();
		HString frameString = HString::FromFloat(frameRate, 2) + " ms";
		HString fpsString = HString::FromUInt((uint32_t)(1.0f / (float)(frameRate / 1000.0)));
		ImVec2 newPos = { (float)_currentFrameBufferSize.width - 80.f , (float)_currentFrameBufferSize.height * 0.0002f };
		ImGui::SetWindowPos(newPos);
		ImGui::Text("%s", frameString.c_str());
		ImGui::NextColumn();
		ImGui::Text("%s" , fpsString.c_str());
		ImGui::End();
	}
}
#pragma endregion ImguiScreenPass 