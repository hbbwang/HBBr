#include "PassDefine.h"
#include "VulkanRenderer.h"
#include "DescriptorSet.h"
#include "VertexFactory.h"
#include "imgui.h"
#include "Buffer.h"
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
	_descriptorSet_mat.reset();
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
			_descriptorSet_obj->GetDescriptorSetLayout(),
			_descriptorSet_mat->GetDescriptorSetLayout()
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
	VkDeviceSize matOffset = 0;
	uint32_t objectUboOffset = 0;
	std::vector<uint32_t> matBufferOffset;
	std::vector<uint32_t> matBufferSize;
	PipelineObject* pipelineObj = NULL;
	//Reset buffer
	{
		bool bUpdateObjUb = false;
		//Update pass uniform buffers
		{
			PassUniformBuffer passUniformBuffer = _renderer->GetPassUniformBufferCache();		
			_descriptorSet_pass->BufferMapping(&passUniformBuffer, 0, sizeof(PassUniformBuffer));
		}
		uint32_t objectCount = 0;
		for (auto m : PrimitiveProxy::GetMaterialPrimitives((uint32_t)Pass::OpaquePass))
		{
			//Get Pipeline
			pipelineObj = PipelineManager::GetGraphicsPipelineMap(m->graphicsIndex);
			if (pipelineObj == NULL)
			{
				auto vsCache = Shader::_vsShader[m->vsShader];
				auto psCache = Shader::_psShader[m->psShader];
				VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
				PipelineManager::SetColorBlend(pipelineCreateInfo, false);
				PipelineManager::SetColorBlend(pipelineCreateInfo, false);
				PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
				PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
				PipelineManager::SetVertexInput(pipelineCreateInfo, m->inputLayout);
				PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache, psCache);
				//Setting pipeline end
				pipelineObj = PipelineManager::CreatePipelineObject(pipelineCreateInfo, _pipelineLayout,
					_renderPass, m->graphicsIndex, (uint32_t)_subpassDescs.size());
			}
			if (m->uniformBufferSize != 0)
			{
				////Test
				//m->uniformBuffer[2] = glm::vec4(1,0,0,1);
				_descriptorSet_mat->BufferMapping(m->uniformBuffer.data(), 0, m->uniformBufferSize);
				matBufferOffset.push_back((uint32_t)matOffset);
				matBufferSize.push_back((uint32_t)m->uniformBufferSize);
				matOffset += m->uniformBufferSize;
			}
			auto prims = PrimitiveProxy::GetModelPrimitives(m);
			for (size_t m = 0; m < prims.size(); m++)
			{
				ModelPrimitive* prim = prims[m];
				//Vb,Ib
				bool bRefreshBuffer = false; 
				if (vbOffset != prim->vbPos) //偏移信息不相同时,重新映射
				{
					_vertexBuffer->BufferMapping(prim->vertexData.data(), vbOffset, prim->vbSize);
					prim->vbPos = vbOffset;
					bRefreshBuffer = true;
				}
				if (ibOffset != prim->ibPos || bRefreshBuffer) //偏移信息不相同时,重新映射
				{
					_indexBuffer->BufferMapping(prim->vertexIndices.data(), ibOffset, prim->ibSize);
					prim->ibPos = ibOffset;
					bRefreshBuffer = true;
				}
				vbOffset += prim->vbSize;
				ibOffset += prim->ibSize;
				//Object uniform
				if (prim->transform && 
					(prim->transform->NeedUpdateUb() 
					|| bRefreshBuffer //当顶点发生改变的时候,UniformBuffer也应该一起更新。
					) )
				{
					ObjectUniformBuffer objectUniformBuffer = {};
					objectUniformBuffer.WorldMatrix = prim->transform->GetWorldMatrix();
					_descriptorSet_obj->BufferMapping(&objectUniformBuffer, (uint64_t)objectUboOffset, sizeof(ObjectUniformBuffer));
					bUpdateObjUb = true;
				}
				objectCount++;
				objectUboOffset += (uint32_t)sizeof(ObjectUniformBuffer);
			}
		}
		if (bUpdateObjUb)
		{
			_descriptorSet_obj->ResizeDescriptorBuffer(sizeof(ObjectUniformBuffer) * (objectCount + 1));
			_descriptorSet_obj->UpdateDescriptorSet(sizeof(ObjectUniformBuffer));
		}
		_descriptorSet_mat->ResizeDescriptorBuffer(matOffset);
		_descriptorSet_mat->UpdateDescriptorSet(matBufferSize, matBufferOffset);
	}

	//Update Render
	vbOffset = 0;
	ibOffset = 0;
	objectUboOffset = 0;
	uint32_t dynamicOffset[] = { 0 };
	uint32_t matIndex = 0;
	vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet_pass->GetDescriptorSet(), 1, dynamicOffset);
	for (auto m : PrimitiveProxy::GetMaterialPrimitives((uint32_t)Pass::OpaquePass))
	{
		manager->CmdCmdBindPipeline(cmdBuf, pipelineObj->pipeline);
		vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 2, 1, &_descriptorSet_mat->GetDescriptorSet(), 1, &matBufferOffset[matIndex]);
		auto prims = PrimitiveProxy::GetModelPrimitives(m);
		for (size_t m = 0; m < prims.size(); m++)
		{
			ModelPrimitive* prim = prims[m];
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_descriptorSet_obj->GetDescriptorSet(), 1, &objectUboOffset);
				objectUboOffset += sizeof(ObjectUniformBuffer);

				VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
				vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);
				vkCmdBindIndexBuffer(cmdBuf, _indexBuffer->GetBuffer(), ibOffset, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cmdBuf, (uint32_t)prim->vertexIndices.size(), 1, 0, 0, 0);

				vbOffset += prim->vbSize;
				ibOffset += prim->ibSize;
			}
		}
		matIndex++;
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
