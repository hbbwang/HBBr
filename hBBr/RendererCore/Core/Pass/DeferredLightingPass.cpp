#include "DeferredLightingPass.h"
#include "VulkanRenderer.h"
#include "SceneTexture.h"
#include "Pass/PassManager.h"
#include <stdio.h>

DeferredLightingPass::~DeferredLightingPass()
{
	auto manager = VulkanManager::GetManager();
	manager->DestroyPipelineLayout(_pipelineLayout);
	manager->DestroyDescriptorSetLayout(_texDescriptorSetLayout);
}

void DeferredLightingPass::PassInit()
{
	const auto& manager = VulkanManager::GetManager();
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture(SceneTextureDesc::SceneColor)->GetFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, -1,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	CreateRenderPass();
	//DescriptorSet
	//SceneDepth,GBuffer0,GBuffer1,GBuffer2
	manager->CreateDescripotrSetLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, _texDescriptorSetLayout, VK_SHADER_STAGE_FRAGMENT_BIT);
	manager->CreatePipelineLayout(
		{
			PipelineManager::GetDescriptorSetLayout_UniformBufferDynamicPS(),
			_texDescriptorSetLayout,
		}
	, _pipelineLayout);
	_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	vertices =
	{
		{ glm::vec2(-1.0f, -1.0f)	, glm::vec2(0.0f, 0.0f) },
		{ glm::vec2(1.0f, -1.0f)	, glm::vec2(1.0f, 0.0f) },
		{ glm::vec2(-1.0f, 1.0f)	, glm::vec2(0.0f, 1.0f) },
		{ glm::vec2(1.0f, -1.0f)	, glm::vec2(1.0f, 0.0f) },
		{ glm::vec2(1.0f, 1.0f)		, glm::vec2(1.0f, 1.0f) },
		{ glm::vec2(-1.0f, 1.0f)	, glm::vec2(0.0f, 1.0f) }
	};
	_vertexBuffer->BufferMapping(vertices.data(), 0, sizeof(LightingVertexData) * vertices.size());
	_vertexBuffer->UnMapMemory();
	//DescriptorSet
	auto bufferSize = sizeof(LightingUniformBuffer);
	_ub_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, PipelineManager::GetDescriptorSetLayout_UniformBufferDynamicPS(), bufferSize, VK_SHADER_STAGE_FRAGMENT_BIT));
	_tex_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _texDescriptorSetLayout, 0, VK_SHADER_STAGE_FRAGMENT_BIT));
	_ub_descriptorSet->UpdateDescriptorSetAll((uint32_t)bufferSize);
	//Set Pass Name
	_passName = "Lighting Pass"; 
}

void DeferredLightingPass::PassUpdate()
{
	const auto& manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.5, 0.6, 0.7, 0.3));
	//Update FrameBuffer
	ResetFrameBufferCustom(_renderer->GetRenderSize(), 
		{
			GetSceneTexture(SceneTextureDesc::SceneColor)->GetTextureView()
		});
	SetViewport(_renderer->GetRenderSize());

	auto sceneDepth = GetSceneTexture(SceneTextureDesc::SceneDepth);
	auto gbuffer0 = GetSceneTexture(SceneTextureDesc::GBuffer0);
	auto gbuffer1 = GetSceneTexture(SceneTextureDesc::GBuffer1);
	auto gbuffer2 = GetSceneTexture(SceneTextureDesc::GBuffer2);

	BeginRenderPass({ 0,0,0,0 });
	//Begin...

	//设置管线
	//Create GUIShader Pipeline
	auto pipelineObject = PipelineManager::GetGraphicsPipelineMap(_shaderIndex);
	if (pipelineObject == nullptr)
	{
		_shaderIndex = CreatePipeline("DeferredLighting@0");
	}

	//vertex buffer
	VkDeviceSize vbOffset = 0;
	VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
	vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);

	//textures
	_tex_descriptorSet->UpdateTextureDescriptorSet(
		{
			sceneDepth.get(),
			gbuffer0.get(),
			gbuffer1.get(),
			gbuffer2.get()
		}, 
		{ 
			Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp),
			Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp),
			Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp),
			Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp),
		});

	//uniform buffers
	uint32_t ubSize = (uint32_t)manager->GetMinUboAlignmentSize(sizeof(LightingUniformBuffer));
	auto uniformBuffer = _manager->GetLightingUniformBuffer();
	_ub_descriptorSet->BufferMapping(uniformBuffer, 0, ubSize);
	_ub_descriptorSet->NeedUpdate();
	_ub_descriptorSet->UpdateDescriptorSet(sizeof(LightingUniformBuffer));
	//Pipeline
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipeline currentPipeline = PipelineManager::GetGraphicsPipelineMap(_shaderIndex)->pipeline;
	if (currentPipeline != pipeline)
	{
		pipeline = currentPipeline;
		manager->CmdCmdBindPipeline(cmdBuf, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	}
	//textures
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_tex_descriptorSet->GetDescriptorSet(), 0, 0);
	//uniform buffers
	uint32_t ubOffset = 0;
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_ub_descriptorSet->GetDescriptorSet(), 1, &ubOffset);
	//draw primitive
	vkCmdDraw(cmdBuf, 6, 1, 0, 0);
	//End...
	EndRenderPass();
}

void DeferredLightingPass::PassReset()
{
	_tex_descriptorSet->NeedUpdate();
}

PipelineIndex DeferredLightingPass::CreatePipeline(HString shaderName)
{
	//CraetePipeline..
	VkPipeline pipeline = VK_NULL_HANDLE;
	auto vsCache = Shader::_vsShader[shaderName];
	auto psCache = Shader::_psShader[shaderName];
	VertexInputLayout vertexInputLayout = {};

	vertexInputLayout.inputLayouts.resize(2);
	vertexInputLayout.inputLayouts[0] = VK_FORMAT_R32G32_SFLOAT;//Pos
	vertexInputLayout.inputLayouts[1] = VK_FORMAT_R32G32_SFLOAT;//UV
	vertexInputLayout.inputSize = sizeof(float) * 4;

	vertexInputLayout.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
	PipelineManager::SetColorBlend(pipelineCreateInfo, true,
		StaticBlendState(
			1,
			CW_RGBA,
			BO_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA,//color
			BO_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA //alpha
		));
	PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
	//PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
	PipelineManager::SetVertexInput(pipelineCreateInfo, vertexInputLayout);
	PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache.get(), psCache.get());
	PipelineManager::SetPipelineLayout(pipelineCreateInfo, _pipelineLayout);
	PipelineManager::BuildGraphicsPipelineState(pipelineCreateInfo, _renderPass, 0, pipeline);
	//_guiPipelines.emplace(std::make_pair(pipelineTag, pipeline));
	return PipelineManager::AddPipelineObject(vsCache, psCache, pipeline, _pipelineLayout);
}
