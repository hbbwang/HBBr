#include "ScreenshotPass.h"
#include "VulkanRenderer.h"

//截图Pass单独出来，是为了能做一些特殊处理，比如锐化，或者超级分辨率之类的，不过这都是后话了。

bool ScreenshotPass::bScreenshot = false;

ScreenshotPass::~ScreenshotPass()
{
	auto manager = VulkanManager::GetManager();
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout);
}

void ScreenshotPass::PassInit()
{
	const auto& manager = VulkanManager::GetManager();
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, { 0 }, -1,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	CreateRenderPass();
	//DescriptorSet
	manager->CreateDescripotrSetLayout({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC }, _ubDescriptorSetLayout, VK_SHADER_STAGE_FRAGMENT_BIT);
	manager->CreateDescripotrSetLayout({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, _texDescriptorSetLayout, VK_SHADER_STAGE_FRAGMENT_BIT);
	_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	manager->CreatePipelineLayout(
		{
			_ubDescriptorSetLayout,
			_texDescriptorSetLayout,
		}
	, _pipelineLayout);

	_ub_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _ubDescriptorSetLayout, 1, sizeof(ScreenshotUniformBuffer), VK_SHADER_STAGE_FRAGMENT_BIT));
	_tex_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _texDescriptorSetLayout, 1, 0, VK_SHADER_STAGE_FRAGMENT_BIT));
	_ub_descriptorSet->UpdateDescriptorSet(sizeof(ScreenshotUniformBuffer));

	//Set Pass Name
	_passName = "Screenshot Pass";
	_markColor = glm::vec4(0.3, 0.3, 0.35, 0.5);
}

void ScreenshotPass::PassUpdate()
{
	if (!bScreenshot)
	{
		return;
	}

	const auto& manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.3, 1.0, 0.1, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), {});
	SetViewport(_currentFrameBufferSize);

	auto sceneColor = GetSceneTexture((uint32_t)SceneTextureDesc::SceneColor).get();
	sceneColor->Transition(cmdBuf, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	BeginRenderPass({ 0,0,0,0 });
	//Begin...

	//设置管线
	//Create GUIShader Pipeline
	auto pipelineObject = PipelineManager::GetGraphicsPipelineMap(_shaderIndex);
	if (pipelineObject == nullptr)
	{
		_shaderIndex = CreatePipeline("Screenshot@0");
	}
	//收集顶点数据一次性使用
	//vertex buffer
	std::vector<ScreenshotVertexData> vertices =
	{
		{ glm::vec2(0.0f, 0.0f) , glm::vec2(0.0f, 0.0f) },
		{ glm::vec2(1.0f, 0.0f) , glm::vec2(1.0f, 0.0f) },
		{ glm::vec2(0.0f, 1.0f) , glm::vec2(0.0f, 1.0f) },
		{ glm::vec2(1.0f, 0.0f) , glm::vec2(1.0f, 0.0f) },
		{ glm::vec2(1.0f, 1.0f) , glm::vec2(1.0f, 1.0f) },
		{ glm::vec2(0.0f, 1.0f) , glm::vec2(0.0f, 1.0f) }
	};
	//textures
	_tex_descriptorSet->UpdateTextureDescriptorSet({ sceneColor }, { Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Wrap) });
	uint32_t ubSize = (uint32_t)manager->GetMinUboAlignmentSize(sizeof(ScreenshotUniformBuffer));
	//uniform buffers
	ScreenshotUniformBuffer uniform = {};
	_ub_descriptorSet->BufferMapping(&uniform, 0, ubSize);
	//vertex buffer
	_vertexBuffer->BufferMapping(vertices.data(), 0, sizeof(ScreenshotVertexData) * vertices.size());
	VkDeviceSize vbOffset = 0;
	VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
	vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);
	//Pipeline
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipeline currentPipeline = PipelineManager::GetGraphicsPipelineMap(_shaderIndex)->pipeline;
	if (currentPipeline != pipeline)
	{
		pipeline = currentPipeline;
		manager->CmdCmdBindPipeline(cmdBuf, pipeline);
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

	sceneColor->Transition(cmdBuf, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void ScreenshotPass::PassReset()
{
}

PipelineIndex ScreenshotPass::CreatePipeline(HString shaderName)
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
