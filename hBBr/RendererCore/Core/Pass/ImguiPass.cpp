#include "ImguiPass.h"
#include "VulkanRenderer.h"
#include "imgui.h"
#include "SceneTexture.h"
#include "FormMain.h"
#include "PassManager.h"
#include "VulkanSwapchain.h"
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
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSwapchain()->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, -1);
	CreateRenderPass();
	_imguiContent = VulkanManager::GetManager()->InitImgui_SDL(_renderer->GetSwapchain()->GetWindowHandle(), _renderPass, false, false);
	auto& forms = VulkanApp::GetForms();
	for (auto& i : VulkanApp::GetForms())
	{
		if (i->swapchain == _swapchain)
		{
			i->imguiContents.push_back(_imguiContent);
		}
	}
	_passName = "Imgui Render Pass";

	bSpawnOutputRT = false;
}


ImguiPass::ImguiPass(PassManager* manager) :GraphicsPass(manager) 
{
	_swapchain = _renderer->GetSwapchain();
}

ImguiPass::~ImguiPass()
{
	ImGui::SetCurrentContext(_imguiContent);
	_renderView.reset();
	_renderViewTexture.reset();
	VulkanManager::GetManager()->ShutdownImgui();
	if (_imguiContent != nullptr)
		ImGui::DestroyContext(_imguiContent);
}

void ImguiPass::PassReset()
{
}

void ImguiPass::EndFrame()
{
	auto* vkManager = VulkanManager::GetManager();
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

void ImguiPass::EnableOutputRT(bool bEnable)
{
	bSpawnOutputRT = bEnable;
	if (!bSpawnOutputRT)
	{
		_renderView.reset();
		_renderViewTexture.reset();
	}
}

VkDescriptorSet ImguiPass::GetRenderView() const
{
	return _renderView->GetDescriptorSet();
}

void ImguiPass::PassUpdate()
{
	if (ImGui::GetCurrentContext() != _imguiContent)
	{
		ImGui::SetCurrentContext(_imguiContent);
	}

	auto* vkManager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.1, 0.4, 0.2, 0.2));
	//
	auto renderSize = _renderer->GetRenderSize();
	if (bSpawnOutputRT)
	{
		auto finalColor = GetSceneTexture(SceneTextureDesc::FinalColor);
		if (!_renderView)
		{
			_renderView.reset(new DescriptorSet(_renderer));
			_renderView->CreateBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			_renderView->BuildDescriptorSetLayout();
		}
		if (!_renderViewTexture)
		{
			_renderViewTexture = Texture2D::CreateTexture2D(
				1,
				1,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				"RenderOutputFromImguiPass");
		}
		else if (
			renderSize.width != _renderViewTexture->GetTextureSize().width ||
			renderSize.height != _renderViewTexture->GetTextureSize().height
			)
		{
			vkManager->DeviceWaitIdle();
			_renderViewTexture->Resize(renderSize.width, renderSize.height);
			VulkanObjectManager::Get()->RefreshTexture(_renderViewTexture);
		}
		finalColor->Transition(cmdBuf, finalColor->GetLayout(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		_renderViewTexture->Transition(cmdBuf, _renderViewTexture->GetLayout(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkManager->CmdColorBitImage(cmdBuf, finalColor->GetTexture(), _renderViewTexture->GetTexture(),
			{ renderSize.width,renderSize.height },
			{ renderSize.width,renderSize.height },
			VK_FILTER_NEAREST);
		finalColor->Transition(cmdBuf, finalColor->GetLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		_renderViewTexture->Transition(cmdBuf, _renderViewTexture->GetLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		_renderView->UpdateTextureDescriptorSet({ {
			_renderViewTexture,
			Texture2D::GetSampler(TextureSampler_Nearest_Clamp)
		} });
	}
	//Update FrameBuffer
	ResetFrameBufferCustom(renderSize, { GetSceneTexture(SceneTextureDesc::FinalColor) });
	SetViewport(renderSize);
	BeginRenderPass({ 0,0,0,0 });

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 transformedMousePos;
	transformedMousePos.x = (HInput::GetMousePos().x - _focusRect._min.x);
	transformedMousePos.y = (HInput::GetMousePos().y - _focusRect._min.y);
	io.MousePos = transformedMousePos;
	io.WantOffsetMousePos = true;//偏移光标位置

	vkManager->ImguiNewFrame();
	//Begin
	
	ImGui::ShowDemoWindow((bool*)1);
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
		ImGuiWindowFlags_NoMove | 
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		auto frameRate = VulkanApp::GetFrameRate();
		uint32_t fps = (uint32_t)(1.0f / (float)(frameRate / 1000.0));
		ImVec2 newPos = { 2.0f , 2.0f };
		ImGui::SetWindowPos(newPos);
		ImGui::Text("%.2f ms", frameRate);
		ImGui::NextColumn();
		ImGui::Text("%d" , fps);
		ImGui::End();
	}
}
#pragma endregion ImguiPass 