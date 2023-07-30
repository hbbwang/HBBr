#include "PassDefine.h"
#include "VulkanRenderer.h"
#include "DescriptorSet.h"
#include "VertexFactory.h"
#include "imgui.h"

/*
	Opaque pass
*/
OpaquePass::~OpaquePass()
{
	_descriptorSet_pass.reset();
	_descriptorSet_obj.reset();
}

void OpaquePass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, { 0 }, -1);
	CreateRenderPass();
}

void OpaquePass::PassBuild()
{
	_descriptorSet_pass.reset(new DescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_descriptorSet_pass->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	_descriptorSet_obj.reset(new DescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_descriptorSet_obj->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	_pipeline.reset(new Pipeline());
	//Setting pipeline start
	_pipeline->SetColorBlend(false);
	_pipeline->SetRenderRasterizer();
	_pipeline->SetRenderDepthStencil();
	_pipeline->SetVertexInput(VertexFactory::VertexInputBase::BuildLayout());
	//
	_pipeline->SetPipelineLayout(
		{
			_descriptorSet_pass->GetDescriptorSetLayout() ,
			_descriptorSet_obj->GetDescriptorSetLayout()
		});
	_pipeline->SetVertexShaderAndPixelShader(Shader::_vsShader["TestShader"], Shader::_psShader["TestShader"]);
	//Setting pipeline end
	_pipeline->CreatePipelineObject(_renderPass, (uint32_t)_subpassDescs.size());
}

void OpaquePass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), _renderer->GetSwapchainImageViews(), {});
	manager->CmdSetViewport(cmdBuf, { _currentFrameBufferSize });
	manager->BeginRenderPass(cmdBuf, GetFrameBuffer(), _renderPass, _currentFrameBufferSize, _attachmentDescs, { 0,0,0.0,0 });

	//vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->GetPipelineObject());



	manager->EndRenderPass(cmdBuf);
}

/*
	Imgui pass
*/

void ImguiPass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, { 0 }, -1);
	CreateRenderPass();
}
void ImguiPass::PassBuild()
{
	const auto manager = VulkanManager::GetManager();
	manager->InitImgui(_renderer->GetWindowHandle(), _renderPass);
}

ImguiPass::~ImguiPass()
{
	VulkanManager::GetManager()->ShutdownImgui();
}

void ImguiPass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), _renderer->GetSwapchainImageViews(), {});
	manager->CmdSetViewport(cmdBuf, { _currentFrameBufferSize });
	manager->BeginRenderPass(cmdBuf, GetFrameBuffer(), _renderPass, _currentFrameBufferSize, _attachmentDescs, { 0,0,0,0 });
	//Begin
	manager->ImguiNewFrame();
	ShowPerformance();
	//End
	manager->ImguiEndFrame(cmdBuf);
	manager->EndRenderPass(cmdBuf);
}

void ImguiPass::ShowPerformance()
{
	if (ImGui::Begin("Performance", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove))
	{
		double frameRate = _renderer->GetFrameRate();
		HString frameString = HString::FromFloat(frameRate, 2) + " ms";
		HString fpsString = HString::FromUInt((uint32_t)(1.0f / (float)(frameRate / 1000.0)));
		ImVec2 newPos = { (float)_currentFrameBufferSize.width - 80.f , (float)_currentFrameBufferSize.height * 0.0002f };
		ImGui::SetWindowPos(newPos);
		ImGui::Text(frameString.c_str());
		ImGui::NextColumn();
		ImGui::Text(fpsString.c_str());
		ImGui::End();
	}
}
