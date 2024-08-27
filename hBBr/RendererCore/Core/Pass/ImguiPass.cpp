#include "ImguiPass.h"
#include "VulkanRenderer.h"
#include "imgui.h"
#include "SceneTexture.h"
#include "FormMain.h"
#include "PassManager.h"
/*
	Imgui buffer pass 
*/

#if ENABLE_IMGUI
#include "Imgui/imgui.h"
#include "Imgui/backends/imgui_impl_sdl3.h"
#include "Imgui/backends/imgui_impl_vulkan.h"
#endif

#pragma region ImguiPass 
void ImguiPass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, -1);
	CreateRenderPass();
	_imguiContent = VulkanManager::GetManager()->InitImgui_SDL(_renderer->GetWindowHandle(), _renderPass, false, false);
	auto& forms = VulkanApp::GetForms();
	for (auto& i : forms)
	{
		if (i->renderer == _renderer)
		{
			i->imguiContents.push_back(_imguiContent);
		}
	}
	_passName = "Imgui Render Pass";
}

ImguiPass::~ImguiPass()
{
	ImGui::SetCurrentContext(_imguiContent);
	VulkanManager::GetManager()->ShutdownImgui();
	if (_imguiContent != nullptr)
		ImGui::DestroyContext(_imguiContent);
}

void ImguiPass::PassReset()
{
}

void ImguiPass::EndFrame()
{
	const auto& vkManager = VulkanManager::GetManager();
	if (ImGui::GetCurrentContext() != _imguiContent)
	{
		ImGui::SetCurrentContext(_imguiContent);
	}
	vkManager->ImguiEndFrame(nullptr);
}

void ImguiPass::AddGui(std::function<void(struct ImGuiContext*)> fun)
{
	_gui_extensions.push_back(fun);
}

void ImguiPass::PassUpdate()
{
	if (ImGui::GetCurrentContext() != _imguiContent)
	{
		ImGui::SetCurrentContext(_imguiContent);
	}
	const auto& vkManager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.1, 0.4, 0.2, 0.2));
	//Update FrameBuffer
	ResetFrameBufferCustom(_renderer->GetRenderSize(), { GetSceneTexture(SceneTextureDesc::FinalColor) });
	SetViewport(_renderer->GetRenderSize());
	BeginRenderPass({ 0,0,0,0 });
	vkManager->ImguiNewFrame();
	//Begin
	//ImGui::ShowDemoWindow((bool*)1);
	for (auto& i : _gui_extensions)
	{
		i(_imguiContent);
	}
	ShowPerformance();

	//End
	vkManager->ImguiEndDraw(cmdBuf);
	EndRenderPass();
}

void ImguiPass::ShowPerformance()
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
#pragma endregion ImguiPass 