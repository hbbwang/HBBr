#include "PostProcessPass.h"
#include "VulkanRenderer.h"
#include "SceneTexture.h"
#include "Pass/PassManager.h"
#include <stdio.h>

PostProcessPass::~PostProcessPass()
{
	auto manager = VulkanManager::GetManager();
	manager->DestroyPipelineLayout(_pipelineLayout);
}

void PostProcessPass::PassInit()
{
	auto* manager = VulkanManager::GetManager();
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture(SceneTextureDesc::FinalColor)->GetFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, -1,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	CreateRenderPass();
	//DescriptorSet
	//SceneDepth,SceneColor,GBuffer0,GBuffer1,GBuffer2
	vertices =
	{
		{ glm::vec2(-1.0f, -1.0f)	, glm::vec2(0.0f, 0.0f) },
		{ glm::vec2(1.0f, -1.0f)	, glm::vec2(1.0f, 0.0f) },
		{ glm::vec2(-1.0f, 1.0f)	, glm::vec2(0.0f, 1.0f) },
		{ glm::vec2(1.0f, -1.0f)	, glm::vec2(1.0f, 0.0f) },
		{ glm::vec2(1.0f, 1.0f)		, glm::vec2(1.0f, 1.0f) },
		{ glm::vec2(-1.0f, 1.0f)	, glm::vec2(0.0f, 1.0f) }
	};

	//后处理永远都使用的是屏幕面片，不会发生改变，所以用 VMA_MEMORY_USAGE_GPU_ONLY
	_vertexBuffer.reset(new VMABuffer(sizeof(PostProcessVertexData) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, false, false, "PostProcessPass_Vb"));
	_vertexBuffer->Mapping(vertices.data(), 0, sizeof(PostProcessVertexData) * vertices.size());

	//DescriptorSet
	auto bufferSize = sizeof(PostProcessUniformBuffer);
	_ub_descriptorSet.reset(new DescriptorSet(_renderer));
	_ub_descriptorSet->CreateBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT);
	_ub_descriptorSet->CreateBuffer(0, bufferSize, VMA_MEMORY_USAGE_CPU_TO_GPU, TRUE, FALSE, "PostProcessPass_Ub");
	_ub_descriptorSet->BuildDescriptorSetLayout();

	_tex_descriptorSet.reset(new DescriptorSet(_renderer));
	_tex_descriptorSet->CreateBindings(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	_tex_descriptorSet->BuildDescriptorSetLayout();

	manager->CreatePipelineLayout(
		{
			_ub_descriptorSet->GetLayout(),
			_tex_descriptorSet->GetLayout(),
		}
	, _pipelineLayout);
}

void PostProcessPass::PassUpdate()
{
	auto* manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.7, 0.6, 0.6, 0.4));

	auto finalColor = GetSceneTexture(SceneTextureDesc::FinalColor);
	finalColor->Transition(cmdBuf, finalColor->GetLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//Update FrameBuffer
	ResetFrameBufferCustom(_renderer->GetRenderSize(), 
		{
			finalColor
		});
	SetViewport(_renderer->GetRenderSize());

	auto sceneDepth = GetSceneTexture(SceneTextureDesc::SceneDepth);
	auto sceneColor = GetSceneTexture(SceneTextureDesc::SceneColor);
	auto gbuffer0 = GetSceneTexture(SceneTextureDesc::GBuffer0);
	auto gbuffer1 = GetSceneTexture(SceneTextureDesc::GBuffer1);
	auto gbuffer2 = GetSceneTexture(SceneTextureDesc::GBuffer2);

	if (sceneColor->GetLayout() == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		sceneColor->Transition(cmdBuf, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	BeginRenderPass({ 0,0,0,0 });
	//Begin...

	//设置管线
	//Create GUIShader Pipeline
	auto pipelineObject = PipelineManager::GetGraphicsPipelineMap(_shaderIndex);
	if (pipelineObject == nullptr)
	{
		_shaderIndex = CreatePipeline("PostProcess@0");
	}

	//vertex buffer
	VkDeviceSize vbOffset = 0;
	VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
	vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);

	//textures
	auto sampler = Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp);
	_tex_descriptorSet->UpdateTextureDescriptorSet({
		{sceneDepth,sampler},
		{gbuffer0,sampler},
		{gbuffer1,sampler},
		{gbuffer2,sampler},
		{sceneColor,sampler},
	});
		 
	//uniform buffers
	uint32_t ubSize = (uint32_t)manager->GetMinUboAlignmentSize(sizeof(PostProcessUniformBuffer));
	auto uniformBuffer = _manager->GetPostProcessUniformBuffer();
	uniformBuffer->passUniform = _manager->GetPassUniformBufferCache(); 
	_ub_descriptorSet->GetBuffer(0)->Mapping(uniformBuffer, 0, ubSize);
	_ub_descriptorSet->UpdateDescriptorSetWholeBuffer(0);
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

void PostProcessPass::PassReset()
{
	_tex_descriptorSet->RefreshDescriptorSet(0);
}

PipelineIndex PostProcessPass::CreatePipeline(HString shaderName)
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
	PipelineManager::SetColorBlend(pipelineCreateInfo, false);
	PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
	//PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
	PipelineManager::SetVertexInput(pipelineCreateInfo, vertexInputLayout);
	PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache.get(), psCache.get());
	PipelineManager::SetPipelineLayout(pipelineCreateInfo, _pipelineLayout);
	PipelineManager::BuildGraphicsPipelineState(pipelineCreateInfo, _renderPass, 0, pipeline);
	//_guiPipelines.emplace(std::make_pair(pipelineTag, pipeline));
	return PipelineManager::AddPipelineObject(vsCache, psCache, pipeline, _pipelineLayout);
}
