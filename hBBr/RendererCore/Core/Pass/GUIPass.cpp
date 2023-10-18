#include "GUIPass.h"
#include "VulkanRenderer.h"

GUIVertexData _guiPlane[6] = {
	{glm::vec2(-0.5f,-0.5f),glm::vec2(0.0f, 0.0f),glm::vec4(1)},	// 左下角
	{glm::vec2(0.5f, -0.5f),glm::vec2(1.0f, 0.0f),glm::vec4(1)},	// 右下角
	{glm::vec2(-0.5f,  0.5f),glm::vec2(0.0f, 1.0f),glm::vec4(1)},	// 左上角
	{glm::vec2(0.5f, -0.5f),glm::vec2(1.0f, 0.0f),glm::vec4(1)},	// 右下角
	{glm::vec2(0.5f,  0.5f),glm::vec2(1.0f, 1.0f),glm::vec4(1)},	// 右上角
	{glm::vec2(-0.5f,  0.5f),glm::vec2(0.0f, 1.0f),glm::vec4(1)},	// 左上角
};

GUIPass::~GUIPass()
{
	_descriptorSet.reset();
	_vertexBuffer.reset();
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout);
	for (auto i : _guiPipelines)
	{
		VulkanManager::GetManager()->DestroyPipeline(i.second);
	}
	_guiPipelines.clear();
}

void GUIPass::PassInit()
{
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, { 0 }, -1 , 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	CreateRenderPass();
	//DescriptorSet
	_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, sizeof(GUIUniformBuffer)));
	_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	VulkanManager::GetManager()->CreatePipelineLayout(
		{
			_descriptorSet->GetDescriptorSetLayout() ,
		}
	, _pipelineLayout);
	_descriptorSet->UpdateDescriptorSetAll(sizeof(GUIUniformBuffer));
	//Set Pass Name
	_passName = "GUI Render Pass";
}

void GUIPass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.3, 1.0, 0.1, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), {});
	SetViewport(_currentFrameBufferSize);
	BeginRenderPass({ 0,0,0,0 });
	//Begin...
	AddImage(0, 0, 300, 300, GUIAnchor::TopLeft);

	uint32_t dynamic_offset[1] = { (uint32_t)0 };
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet->GetDescriptorSet(), 1, dynamic_offset);
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkDeviceSize vbOffset = 0;
	for (auto i : _drawList)
	{
		VkPipeline currentPipeline = _guiPipelines[i.PipelineTag];
		if (currentPipeline != pipeline)
		{
			pipeline = currentPipeline;
			manager->CmdCmdBindPipeline(cmdBuf, pipeline);
		}
		VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
		vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);
		vkCmdDraw(cmdBuf, i.Data.size(), 1, 0, 0);
	}

	//End...
	EndRenderPass();
	_drawList.clear();
}

void GUIPass::PassReset()
{

}

void GUIPass::AddImage(float lx, float ly, float w, float h , GUIAnchor anchor)
{
	GUIDrawState state;
	state.Data.resize(6);
	state.Data[0] = { glm::vec2(-0.5f,-0.5f),glm::vec2(0.0f, 0.0f),glm::vec4(1) };// 左下角
	state.Data[1] = { glm::vec2(0.5f, -0.5f),glm::vec2(1.0f, 0.0f),glm::vec4(1) };// 右下角
	state.Data[2] = { glm::vec2(-0.5f,  0.5f),glm::vec2(0.0f, 1.0f),glm::vec4(1) };// 左上角
	state.Data[3] = { glm::vec2(0.5f, -0.5f),glm::vec2(1.0f, 0.0f),glm::vec4(1) };// 右下角
	state.Data[4] = { glm::vec2(0.5f,  0.5f),glm::vec2(1.0f, 1.0f),glm::vec4(1) };// 右上角
	state.Data[5] = { glm::vec2(-0.5f,  0.5f),glm::vec2(0.0f, 1.0f),glm::vec4(1) };// 左上角

	state.Anchor = anchor;
	auto it = _guiPipelines.find("Image");
	if (it == _guiPipelines.end())
	{
		state.PipelineTag = "Image";
		//CraetePipeline..
		VkPipeline pipeline = VK_NULL_HANDLE;
		auto vsCache = Shader::_vsShader["GUIShader"];
		auto psCache = Shader::_psShader["GUIShader"];
		VertexInputLayout vertexInputLayout = {};
		vertexInputLayout.inputLayouts.resize(3);
		vertexInputLayout.inputLayouts[0] = VK_FORMAT_R32G32_SFLOAT;//Pos
		vertexInputLayout.inputLayouts[1] = VK_FORMAT_R32G32_SFLOAT;//UV
		vertexInputLayout.inputLayouts[2] = VK_FORMAT_R32G32B32A32_SFLOAT;//Color
		vertexInputLayout.inputSize = sizeof(float) * 8;
		vertexInputLayout.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
		PipelineManager::SetColorBlend(pipelineCreateInfo, true, 
			StaticBlendState(
				1,ColorWriteMask::CW_RGBA,BlendOperation::BO_ADD,
				BlendFactor::BF_SRC_ALPHA,BlendFactor::BF_ONE,
				BlendOperation::BO_ADD,BlendFactor::BF_ONE,BlendFactor::BF_ONE_MINUS_SRC_ALPHA
				));
		PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
		//PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
		PipelineManager::SetVertexInput(pipelineCreateInfo, vertexInputLayout);
		PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache, psCache);
		PipelineManager::SetPipelineLayout(pipelineCreateInfo, _pipelineLayout);
		PipelineManager::BuildGraphicsPipelineState(pipelineCreateInfo, _renderPass, 0 , pipeline);
		_guiPipelines.emplace(std::make_pair("Image", pipeline));
	}
	_drawList.push_back(state);
}
