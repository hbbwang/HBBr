#include "PassDefine.h"
#include "VulkanRenderer.h"
#include "DescriptorSet.h"
#include "VertexFactory.h"
#include "imgui.h"
#include "Buffer.h"
#include "glm/matrix.hpp"

#include "glm/ext.hpp"
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
	_descriptorSet_pass.reset(new DescriptorSet(_renderer,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_descriptorSet_obj.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	_indexBuffer.reset(new Buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
}

void OpaquePass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, OpaquePass,"Opaque Render Pass", glm::vec4(0.2, 1.0, 0.7, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), _renderer->GetSwapchainImageViews(), {});
	manager->CmdSetViewport(cmdBuf, { _currentFrameBufferSize });
	manager->BeginRenderPass(cmdBuf, GetFrameBuffer(), _renderPass, _currentFrameBufferSize, _attachmentDescs, { 0,0,0.0,0 });

	VertexFactory::VertexInput vertexInput = {};
	vertexInput.pos = 
	{ 
		glm::vec3(1.0f,1.0f,-1.0f) ,
		glm::vec3(1.0f,-1.0f,-1.0f),
		glm::vec3(-1.0f,-1.0f,-1.0f),
		glm::vec3(-1.0f,1.0f,-1.0f),
		glm::vec3(-1.0f,1.0f,1.0f),
		glm::vec3(-1.0f,-1.0f,1.0f),
		glm::vec3(1.0f,-1.0f,1.0f),
		glm::vec3(1.0f,1.0f,1.0f),
	};
	vertexInput.col = 
	{
		glm::vec4(1,0,0,1),
		glm::vec4(0,1,0,1),
		glm::vec4(0,0,1,1),
		glm::vec4(1,0,1,1),
		glm::vec4(1,1,1,1),
		glm::vec4(0,1,1,1),
		glm::vec4(1,1,0,1),
		glm::vec4(1,0,1,1),
	};
	std::vector<uint32_t>ib =
	{
		0,1,2,0,2,3,
		0,3,4,0,4,7,
		4,5,6,4,6,7,
		1,6,5,1,5,2,
		3,2,5,3,5,4,
		7,6,1,7,1,0
	};
	auto vbd = vertexInput.GetData();

	VertexFactory::ScreenTriangleVertexInput scrrenTri;

	auto pipeline = PipelineManager::GetGraphicsPipelineMap("Graphics-TestShader-TestShader");
	if (pipeline == NULL)
	{
		VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
		PipelineManager::SetColorBlend(pipelineCreateInfo, false);
		PipelineManager::SetColorBlend(pipelineCreateInfo, false);
		PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
		PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
		PipelineManager::SetVertexInput(pipelineCreateInfo, vertexInput.BuildLayout());
		PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, Shader::_vsShader["TestShader"], Shader::_psShader["TestShader"]);
		//Setting pipeline end
		pipeline = PipelineManager::CreatePipelineObject(pipelineCreateInfo,
			{
				_descriptorSet_pass->GetDescriptorSetLayout() ,
				_descriptorSet_obj->GetDescriptorSetLayout()
			},
			_renderPass, (uint32_t)_subpassDescs.size());
	}
	manager->CmdCmdBindPipeline(cmdBuf, pipeline->pipeline);
	{
		//Update uniform
		_passUniformBuffer = {};
		_passUniformBuffer.ScreenInfo = glm::vec4((float)_currentFrameBufferSize.width, (float)_currentFrameBufferSize.height, 0.001f, 500.0f);
		_passUniformBuffer.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//_passUniformBuffer.View_Inv = glm::inverse(_passUniformBuffer.View);
		float aspect = (float)_currentFrameBufferSize.width / (float)_currentFrameBufferSize.height;
		_passUniformBuffer.Projection = glm::perspective(glm::radians(90.0f), aspect, 0.001f, 500.0f);
		_passUniformBuffer.Projection[1][1] *= -1;
		//_passUniformBuffer.Projection_Inv = glm::inverse(_passUniformBuffer.Projection);
		_passUniformBuffer.ViewProj = _passUniformBuffer.Projection * _passUniformBuffer.View;
		//_passUniformBuffer.ViewProj_Inv = glm::inverse(_passUniformBuffer.ViewProj);
		_passUniformBuffer.WorldMatrix = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		//
		_descriptorSet_pass->GetBuffer()->BufferMapping(&_passUniformBuffer, sizeof(_passUniformBuffer));
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = _descriptorSet_pass->GetBuffer()->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(PassUniformBuffer);
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = _descriptorSet_pass->GetDescriptorSets()[0];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo; 
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional
		vkUpdateDescriptorSets(manager->GetDevice(), 1, &descriptorWrite, 0, VK_NULL_HANDLE);
		uint32_t dynamicOffset[] = { 0 };
		vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0, 1, _descriptorSet_pass->GetDescriptorSets().data(), 1, dynamicOffset);
		//
		VkDeviceSize vertex_offset[1] = { 0 };
		VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
		_vertexBuffer->BufferMapping(vbd.data(), sizeof(float) * vbd.size());
		_indexBuffer->BufferMapping<uint32_t>(ib.data(), sizeof(uint32_t) * ib.size());
		vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, vertex_offset);
		vkCmdBindIndexBuffer(cmdBuf, _indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuf, ib.size(), 1, 0, 0, 0);
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
	COMMAND_MAKER(cmdBuf, OpaquePass, "Imgui Render Pass", glm::vec4(0.1, 0.4, 0.2, 0.2));
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
