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

void ImguiPassEditor::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
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
	_passName = "Imgui Editor Render Pass";

	_renderView.reset(new DescriptorSet(_renderer));
	_renderView->CreateBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	_renderView->BuildDescriptorSetLayout();

	_renderViewTexture = Texture2D::CreateTexture2D(
		1,
		1,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		"EditorFinalColor");
}

void ImguiPassEditor::CheckWindowValid()
{
	static bool main_window_minimized;
	Uint32 windowFlags = SDL_GetWindowFlags(_renderer->GetWindowHandle());
	bool minimized = (windowFlags & SDL_WINDOW_MINIMIZED) != 0;
	if (minimized != main_window_minimized)
	{
		main_window_minimized = minimized;
		const auto& io = ImGui::GetPlatformIO();
		for (int i = 1; i < io.Viewports.Size; i++)
		{
			ImGuiViewport* viewport = io.Viewports[i];
			if (minimized)
				SDL_HideWindow((SDL_Window*)viewport->PlatformHandle);
			else
				SDL_ShowWindow((SDL_Window*)viewport->PlatformHandle);
		}
	}
}

ImguiPassEditor::ImguiPassEditor(VulkanRenderer* renderer)
	:GraphicsPass(nullptr)
{
	_renderer = renderer;
}

ImguiPassEditor::~ImguiPassEditor()
{
	_renderView.reset();
	ImGui::SetCurrentContext(_imguiContent);
	VulkanManager::GetManager()->ShutdownImgui();
	if (_imguiContent != nullptr)
		ImGui::DestroyContext(_imguiContent);
}

void ImguiPassEditor::PassReset()
{
	auto surfaceSize = _renderer->GetWindowSurfaceSize();
	ResetFrameBuffer(surfaceSize, {}, true);
}

void ImguiPassEditor::EndFrame()
{
	const auto& vkManager = VulkanManager::GetManager();
	if (ImGui::GetCurrentContext() != _imguiContent)
	{
		ImGui::SetCurrentContext(_imguiContent);
	}
	vkManager->ImguiEndFrame(nullptr);
}



void ImguiPassEditor::PassUpdate(std::shared_ptr<Texture2D> finalColor)
{
	const auto& vkManager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.1, 0.4, 0.2, 0.2));

	if (ImGui::GetCurrentContext() != _imguiContent)
	{
		ImGui::SetCurrentContext(_imguiContent);
	}

	auto renderSize = _renderer->GetRenderSize();
	if (_renderViewTexture == nullptr || renderSize.width != _renderViewTexture->GetTextureSize().width ||
		renderSize.height != _renderViewTexture->GetTextureSize().height)
	{
		vkManager->DeviceWaitIdle();
		//_renderViewTexture_old = _renderViewTexture;
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
		Texture2D::GetSampler(TextureSampler_Nearest_Wrap)
	} });

	//Update FrameBuffer
	auto surfaceSize = _renderer->GetWindowSurfaceSize();
	if (surfaceSize.width <= 0 || surfaceSize.height <= 0)
	{
		surfaceSize = { 4,4 };
	}
	ResetFrameBuffer(surfaceSize, {}, false);
	SetViewport(surfaceSize);
	BeginRenderPass({ 0,0,0,0 });
	vkManager->ImguiNewFrame();
	//Begin
	//ImGui::ShowDemoWindow((bool*)1);
	for (auto& i : _gui_extensions)
	{
		i(this);
	}
	//ShowPerformance();
	vkManager->ImguiEndDraw(cmdBuf);
	//End
	//vkManager->ImguiEndFrame(cmdBuf);
	EndRenderPass();
}

void ImguiPassEditor::AddGui(std::function<void(ImguiPassEditor*)> fun)
{
	_gui_extensions.push_back(fun);
}

VkDescriptorSet ImguiPassEditor::GetRenderView() const
{
	return _renderView->GetDescriptorSet();
}

void ImguiPassEditor::ShowPerformance()
{
	if (ImGui::Begin("Performance", nullptr,ImGuiWindowFlags_NoBackground))
	{
		double frameRate = VulkanApp::GetFrameRate();
		HString frameString = HString::FromFloat(frameRate, 2) + " ms";
		HString fpsString = HString::FromUInt((uint32_t)(1.0f / (float)(frameRate / 1000.0)));
		ImGui::Text("%s", frameString.c_str());
		ImGui::NextColumn();
		ImGui::Text("%s", fpsString.c_str());
	}
	ImGui::End();
}
