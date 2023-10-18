#include "GUIPass.h"
#include "VulkanRenderer.h"

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
	AddImage(100, 100 , GUIDrawState(GUIAnchor_TopLeft, false ,glm::vec4(1,1,0,0.35)));

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
		_vertexBuffer->BufferMapping(i.Data.data(), vbOffset, sizeof(GUIUniformBuffer) * i.Data.size());
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

void GUIPass::AddImage(float w, float h , GUIDrawState state)
{
	GUIPrimitive prim;
	prim.Data = GetGUIPanel(state, w, h);
	prim.State = state;
	prim.PipelineTag = "Image";
	auto it = _guiPipelines.find(prim.PipelineTag);
	if (it == _guiPipelines.end())
	{
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
	_drawList.push_back(prim);
}

std::vector<GUIVertexData> GUIPass::GetGUIPanel(GUIDrawState state, float w, float h)
{
	std::vector<GUIVertexData> data;
	data.resize(6);
	glm::vec2 aspect = glm::vec2(_currentFrameBufferSize.width, _currentFrameBufferSize.height);
	{
		data[0] = { glm::vec2(-1.0f,	-1.0f),glm::vec2(0.0f, 0.0f),state.Color };
		data[1] = { glm::vec2( 1.0f,	-1.0f),glm::vec2(1.0f, 0.0f),state.Color };
		data[2] = { glm::vec2(-1.0f,	 1.0f),glm::vec2(0.0f, 1.0f),state.Color };
		data[3] = { glm::vec2( 1.0f,	-1.0f),glm::vec2(1.0f, 0.0f),state.Color };
		data[4] = { glm::vec2( 1.0f,	 1.0f),glm::vec2(1.0f, 1.0f),state.Color };
		data[5] = { glm::vec2(-1.0f,	 1.0f),glm::vec2(0.0f, 1.0f),state.Color };
	}
	//data[0].Pos = glm::vec2(-1.0f, -1.0f);// 左上角
	//data[1].Pos = glm::vec2(1.0f, -1.0f);// 右上角
	//data[2].Pos = glm::vec2(-1.0f, 1.0f);// 左下角
	//data[3].Pos = glm::vec2(1.0f, -1.0f);// 右上角
	//data[4].Pos = glm::vec2(1.0f, 1.0f);// 右下角
	//data[5].Pos = glm::vec2(-1.0f, 1.0f);// 左下角
	if (state.Anchor == GUIAnchor_TopLeft)
	{
		if (state.bFixed)
		{
			glm::vec2 wh = glm::vec2(w,h) / 100.0f;
			wh = wh * 2.0f - 1.0f;
			data[0].Pos = glm::vec2(-1.0f		 , -1.0f);
			data[1].Pos = glm::vec2( 1.0f * wh.x , -1.0f);
			data[2].Pos = glm::vec2(-1.0f		 ,  1.0f * wh.y);
			data[3].Pos = glm::vec2( 1.0f * wh.x , -1.0f);
			data[4].Pos = glm::vec2( 1.0f * wh.x ,  1.0f * wh.y);
			data[5].Pos = glm::vec2(-1.0f		 ,  1.0f * wh.y);
		}
		else
		{
			glm::vec2 wh = glm::vec2(w, h) / aspect;
			wh = wh * 2.0f - 1.0f;
			data[0].Pos = glm::vec2(-1.0f, -1.0f);
			data[1].Pos = glm::vec2(1.0f * wh.x, -1.0f);
			data[2].Pos = glm::vec2(-1.0f, 1.0f * wh.y);
			data[3].Pos = glm::vec2(1.0f * wh.x, -1.0f);
			data[4].Pos = glm::vec2(1.0f * wh.x, 1.0f * wh.y);
			data[5].Pos = glm::vec2(-1.0f, 1.0f * wh.y);
		}
	}
	else if (state.Anchor == GUIAnchor_TopCenter)
	{

	}
	else if (state.Anchor == GUIAnchor_TopRight)
	{

	}
	else if (state.Anchor == GUIAnchor_CenterLeft)
	{

	}
	else if (state.Anchor == GUIAnchor_CenterCenter)
	{
		for (auto& i : data)
		{
			i.Pos *= glm::vec2(w, h);
			if (state.bFixed)
				i.Pos /= 100.0f;
			else
				i.Pos /= aspect;
		}
	}
	else if (state.Anchor == GUIAnchor_CenterRight)
	{

	}
	else if (state.Anchor == GUIAnchor_BottomLeft)
	{

	}
	else if (state.Anchor == GUIAnchor_BottomCenter)
	{

	}
	else if (state.Anchor == GUIAnchor_BottomRight)
	{

	}
	return data;
}
