#include "PassDefine.h"
#include "VulkanRenderer.h"
#include "DescriptorSet.h"
#include "VertexFactory.h"
#include "imgui.h"
#include "Buffer.h"
#include "glm/matrix.hpp"
#include "glm/ext.hpp"
#include "Primitive.h"
#include "Pass/PassType.h"
#include "Texture.h"

/*
	Opaque pass
*/
BasePass::~BasePass()
{
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout);
	_descriptorSet_pass.reset();
	_descriptorSet_obj.reset();
}

void BasePass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	//SceneDepth
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture((uint32_t)SceneTextureDesc::SceneDepth)->GetFormat() , VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, 1);
	CreateRenderPass();
}

void BasePass::PassBuild()
{
	_descriptorSet_pass.reset(new DescriptorSet(_renderer,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, sizeof(PassUniformBuffer)));
	_descriptorSet_obj.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_descriptorSet_mat.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	_indexBuffer.reset(new Buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	VulkanManager::GetManager()->CreatePipelineLayout(
		{
			_descriptorSet_pass->GetDescriptorSetLayout() ,
			_descriptorSet_obj->GetDescriptorSetLayout()
		}
	, _pipelineLayout);
	//Pass Uniform总是一尘不变的,并且我们用的是Dynamic uniform buffer ,所以只需要更新一次所有的DescriptorSet即可。
	_descriptorSet_pass->UpdateDescriptorSetAll(sizeof(PassUniformBuffer));
}

void BasePass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass,"Opaque Render Pass", glm::vec4(0.2, 1.0, 0.7, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), _renderer->GetSwapchainImageViews(), { GetSceneTexture((uint32_t)SceneTextureDesc::SceneDepth)->GetTextureView() });
	manager->CmdSetViewport(cmdBuf, { _currentFrameBufferSize });
	manager->BeginRenderPass(cmdBuf, GetFrameBuffer(), _renderPass, _currentFrameBufferSize, _attachmentDescs, { 0,0,0.0,0 });
	VkDeviceSize vbOffset = 0;
	VkDeviceSize ibOffset = 0;
	uint32_t objectUboOffset = 0;
	//Reset buffer
	{
		//Update pass uniform buffers
		{
			_passUniformBuffer = {};
			_passUniformBuffer.ScreenInfo = glm::vec4((float)_currentFrameBufferSize.width, (float)_currentFrameBufferSize.height, 0.001f, 500.0f);
			_passUniformBuffer.View = glm::lookAt(glm::vec3(0.0f, 2.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			_passUniformBuffer.View_Inv = glm::inverse(_passUniformBuffer.View);
			float aspect = (float)_currentFrameBufferSize.width / (float)_currentFrameBufferSize.height;
			_passUniformBuffer.Projection = glm::perspective(glm::radians(90.0f), aspect, 0.001f, 500.0f);
			_passUniformBuffer.Projection[1][1] *= -1;
			_passUniformBuffer.Projection_Inv = glm::inverse(_passUniformBuffer.Projection);
			_passUniformBuffer.ViewProj = _passUniformBuffer.Projection * _passUniformBuffer.View;
			_passUniformBuffer.ViewProj_Inv = glm::inverse(_passUniformBuffer.ViewProj);
			_descriptorSet_pass->BufferMapping(&_passUniformBuffer, 0, sizeof(_passUniformBuffer));
		}
		uint32_t objectCount = 0;
		for (auto g : PrimitiveProxy::GetMaterialPrimitives((uint32_t)Pass::OpaquePass))
		{
			auto prims = PrimitiveProxy::GetModelPrimitives(g);
			for (size_t m = 0; m < prims.size(); m++)
			{
				ModelPrimitive prim = prims[m];
				if (prim.transform)
					_objectUniformBuffer.WorldMatrix = prim.transform->GetWorldMatrix();
				_descriptorSet_obj->BufferMapping(&_objectUniformBuffer, (uint64_t)objectUboOffset, sizeof(ObjectUniformBuffer), 0);
				VkDeviceSize vbSize = sizeof(float) * prim.vertexData.size();
				VkDeviceSize ibSize = sizeof(uint32_t) * prim.vertexIndices.size();
				_vertexBuffer->BufferMapping(prim.vertexData.data(), vbOffset, vbSize);
				_indexBuffer->BufferMapping(prim.vertexIndices.data(), ibOffset, ibSize);

				objectCount++;
				objectUboOffset += (uint32_t)sizeof(ObjectUniformBuffer);
				vbOffset += vbSize;
				ibOffset += ibSize;
			}
		}
		_descriptorSet_obj->ResizeDescriptorBuffer(sizeof(ObjectUniformBuffer) * (objectCount + 1));
		_descriptorSet_obj->UpdateDescriptorSet(sizeof(ObjectUniformBuffer));
	}

	//Update Render
	vbOffset = 0;
	ibOffset = 0;
	objectUboOffset = 0;
	uint32_t dynamicOffset[] = { 0 };
	vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet_pass->GetDescriptorSet(), 1, dynamicOffset);
	for (auto g : PrimitiveProxy::GetMaterialPrimitives((uint32_t)Pass::OpaquePass))
	{
		auto pipeline = PipelineManager::GetGraphicsPipelineMap(g->graphicsIndex);
		if (pipeline == NULL)
		{
			VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
			PipelineManager::SetColorBlend(pipelineCreateInfo, false);
			PipelineManager::SetColorBlend(pipelineCreateInfo, false);
			PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
			PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
			PipelineManager::SetVertexInput(pipelineCreateInfo, g->inputLayout );
			PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, Shader::_vsShader[g->vsShader], Shader::_psShader[g->psShader]);
			//Setting pipeline end
			pipeline = PipelineManager::CreatePipelineObject(pipelineCreateInfo,_pipelineLayout,
				_renderPass, g->graphicsIndex, (uint32_t)_subpassDescs.size());
		}
		manager->CmdCmdBindPipeline(cmdBuf, pipeline->pipeline);
		auto prims = PrimitiveProxy::GetModelPrimitives(g);
		for (size_t m = 0; m < prims.size(); m++)
		{
			ModelPrimitive prim = prims[m];
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_descriptorSet_obj->GetDescriptorSet(), 1, &objectUboOffset);
				objectUboOffset += sizeof(ObjectUniformBuffer);

				VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
				vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);
				vkCmdBindIndexBuffer(cmdBuf, _indexBuffer->GetBuffer(), ibOffset, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cmdBuf, (uint32_t)prim.vertexIndices.size(), 1, 0, 0, 0);

				vbOffset += sizeof(float) * prim.vertexData.size();
				ibOffset += sizeof(uint32_t) * prim.vertexIndices.size();
			}
		}
	}
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
	COMMAND_MAKER(cmdBuf, BasePass, "Imgui Render Pass", glm::vec4(0.1, 0.4, 0.2, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), _renderer->GetSwapchainImageViews(), {});
	manager->CmdSetViewport(cmdBuf, { _currentFrameBufferSize });
	manager->BeginRenderPass(cmdBuf, GetFrameBuffer(), _renderPass, _currentFrameBufferSize, _attachmentDescs, { 0,0,0,0 });
	manager->ImguiNewFrame();
	//Begin
	//ImGui::ShowDemoWindow((bool*)1);
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
