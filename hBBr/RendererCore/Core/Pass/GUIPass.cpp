#include "GUIPass.h"
#include "VulkanRenderer.h"

GUIPass::~GUIPass()
{
	const auto& manager = VulkanManager::GetManager();
	_descriptorSet.reset();
	_vertexBuffer.reset();
	manager->DestroyPipelineLayout(_pipelineLayout);
	for (auto i : _guiPipelines)
	{
		manager->DestroyPipeline(i.second);
	}
	_drawList.clear();
	_guiPipelines.clear();
	manager->DestroyDescriptorSetLayout(_guiObjectDescriptorSetLayout);
}

void GUIPass::PassInit()
{
	const auto& manager = VulkanManager::GetManager();
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, { 0 }, -1 , 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	CreateRenderPass();
	//DescriptorSet
	manager->CreateDescripotrSetLayout({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, _guiObjectDescriptorSetLayout, VK_SHADER_STAGE_FRAGMENT_BIT);
	_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, sizeof(GUIUniformBuffer)));
	_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	manager->CreatePipelineLayout(
		{
			_descriptorSet->GetDescriptorSetLayout() ,
			_guiObjectDescriptorSetLayout,
		}
	, _pipelineLayout);
	_descriptorSet->UpdateDescriptorSetAll(sizeof(GUIUniformBuffer));
	//Set Pass Name
	_passName = "GUI Render Pass";
}

void GUIPass::PassUpdate()
{
	const auto& manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.3, 1.0, 0.1, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), {});
	SetViewport(_currentFrameBufferSize);
	BeginRenderPass({ 0,0,0,0 });
	//Begin...
	AddImage("TestImage", GUIDrawState(0, 0, 200, 200, GUIAnchor_TopLeft, false, glm::vec4(1, 1, 1, 1), Texture::GetSystemTexture("TestTex")));

	uint32_t dynamic_offset[1] = { (uint32_t)0 };
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet->GetDescriptorSet(), 1, dynamic_offset);
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkDeviceSize vbOffset = 0;
	for (auto i : _drawList)
	{
		VkPipeline currentPipeline = _guiPipelines[i.second.PipelineTag];
		if (currentPipeline != pipeline)
		{
			pipeline = currentPipeline;
			manager->CmdCmdBindPipeline(cmdBuf, pipeline);
		}
		//textures
		i.second._obj_tex_descriptorSet->UpdateTextureDescriptorSet({ i.second.State.BaseTexture });
		vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &i.second._obj_tex_descriptorSet->GetDescriptorSet(), 0, 0);
		//vertex buffer
		_vertexBuffer->BufferMapping(i.second.Data.data(), vbOffset, sizeof(GUIUniformBuffer) * i.second.Data.size());
		VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
		vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);
		//draw primitive
		vkCmdDraw(cmdBuf, i.second.Data.size(), 1, 0, 0);
	}

	//End...
	EndRenderPass();
}

void GUIPass::PassReset()
{

}

void GUIPass::AddImage(HString tag ,GUIDrawState state)
{
	auto dit = _drawList.find(tag);
	GUIPrimitive *prim = NULL;
	if (dit != _drawList.end())
	{
		prim = &dit->second;
		if (prim->State.BaseTexture != state.BaseTexture)
		{
			prim->_obj_tex_descriptorSet->NeedUpdate();
		}
		prim->State = state;
	}
	else
	{
		_drawList.emplace(tag, GUIPrimitive());
		prim = &_drawList[tag];
		prim->PipelineTag = "Image";
		prim->State = state;
	}
	prim->Data = GetGUIPanel(state);
	if (!prim->_obj_tex_descriptorSet)
	{
		prim->_obj_tex_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _guiObjectDescriptorSetLayout, 1, 0, VK_SHADER_STAGE_FRAGMENT_BIT));
	}
	auto it = _guiPipelines.find(prim->PipelineTag);
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
}

std::vector<GUIVertexData> GUIPass::GetGUIPanel(GUIDrawState state)
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

	glm::vec2 xy = state.Translate;
	glm::vec2 wh = state.Scale;
	if (state.bFixed)
	{
		xy /= 100.0f;
		wh /= 100.0f;
	}
	else
	{	
		xy = xy * 2.0f / aspect;
		wh /= aspect;
	}

	if (state.Anchor == GUIAnchor_TopLeft)
	{
		wh = wh * 2.0f - 1.0f;
		data[0].Pos = glm::vec2(-1.0f, -1.0f) + xy;
		data[1].Pos = glm::vec2(1.0f * wh.x, -1.0f) + xy;
		data[2].Pos = glm::vec2(-1.0f, 1.0f * wh.y) + xy;
		data[3].Pos = glm::vec2(1.0f * wh.x, -1.0f) + xy;
		data[4].Pos = glm::vec2(1.0f * wh.x, 1.0f * wh.y) + xy;
		data[5].Pos = glm::vec2(-1.0f, 1.0f * wh.y) + xy;
	}
	else if (state.Anchor == GUIAnchor_TopCenter)
	{
		wh.y = wh.y * 2.0f - 1.0f;
		data[0].Pos = glm::vec2(-1.0f * wh.x, -1.0f) + xy;
		data[1].Pos = glm::vec2(1.0f * wh.x, -1.0f) + xy;
		data[2].Pos = glm::vec2(-1.0f * wh.x, 1.0f * wh.y) + xy;
		data[3].Pos = glm::vec2(1.0f * wh.x, -1.0f) + xy;
		data[4].Pos = glm::vec2(1.0f * wh.x, 1.0f * wh.y) + xy;
		data[5].Pos = glm::vec2(-1.0f * wh.x, 1.0f * wh.y) + xy;
	}
	else if (state.Anchor == GUIAnchor_TopRight)
	{
		wh = wh * 2.0f - 1.0f;
		data[0].Pos = glm::vec2(-1.0f * wh.x, -1.0f) + xy;
		data[1].Pos = glm::vec2(1.0f, -1.0f) + xy;
		data[2].Pos = glm::vec2(-1.0f * wh.x, 1.0f * wh.y) + xy;
		data[3].Pos = glm::vec2(1.0f, -1.0f) + xy;
		data[4].Pos = glm::vec2(1.0f, 1.0f * wh.y) + xy;
		data[5].Pos = glm::vec2(-1.0f * wh.x, 1.0f * wh.y) + xy;
	}
	else if (state.Anchor == GUIAnchor_CenterLeft)
	{
		wh.x = wh.x * 2.0f - 1.0f;
		data[0].Pos = glm::vec2(-1.0f		, -1.0f * wh.y) + xy;
		data[1].Pos = glm::vec2(1.0f * wh.x	, -1.0f * wh.y) + xy;
		data[2].Pos = glm::vec2(-1.0f		,  1.0f * wh.y) + xy;
		data[3].Pos = glm::vec2(1.0f * wh.x	, -1.0f * wh.y) + xy;
		data[4].Pos = glm::vec2(1.0f * wh.x	,  1.0f * wh.y) + xy;
		data[5].Pos = glm::vec2(-1.0f		,  1.0f * wh.y) + xy;
	}
	else if (state.Anchor == GUIAnchor_CenterCenter)
	{
		for (auto& i : data)
		{		
			i.Pos *= wh;
			i.Pos += xy;
		}
	}
	else if (state.Anchor == GUIAnchor_CenterRight)
	{
		wh.x = wh.x * 2.0f - 1.0f;
		data[0].Pos = glm::vec2(-1.0f * wh.x, -1.0f * wh.y) + xy;
		data[1].Pos = glm::vec2( 1.0f		, -1.0f * wh.y) + xy;
		data[2].Pos = glm::vec2(-1.0f * wh.x,  1.0f * wh.y) + xy;
		data[3].Pos = glm::vec2( 1.0f		, -1.0f * wh.y) + xy;
		data[4].Pos = glm::vec2( 1.0f		,  1.0f * wh.y) + xy;
		data[5].Pos = glm::vec2(-1.0f * wh.x,  1.0f * wh.y) + xy;
	}
	else if (state.Anchor == GUIAnchor_BottomLeft)
	{
		wh = wh * 2.0f - 1.0f;
		data[0].Pos = glm::vec2(-1.0f		, -1.0f * wh.y) + xy;
		data[1].Pos = glm::vec2( 1.0f * wh.x, -1.0f * wh.y) + xy;
		data[2].Pos = glm::vec2(-1.0f		,  1.0f) + xy;
		data[3].Pos = glm::vec2( 1.0f * wh.x, -1.0f * wh.y) + xy;
		data[4].Pos = glm::vec2( 1.0f * wh.x,  1.0f) + xy;
		data[5].Pos = glm::vec2(-1.0f		,  1.0f) + xy;
	}
	else if (state.Anchor == GUIAnchor_BottomCenter)
	{
		wh.y = wh.y * 2.0f - 1.0f;
		data[0].Pos = glm::vec2(-1.0f * wh.x, -1.0f * wh.y) + xy;
		data[1].Pos = glm::vec2( 1.0f * wh.x, -1.0f * wh.y) + xy;
		data[2].Pos = glm::vec2(-1.0f * wh.x,  1.0f) + xy;
		data[3].Pos = glm::vec2( 1.0f * wh.x, -1.0f * wh.y) + xy;
		data[4].Pos = glm::vec2( 1.0f * wh.x,  1.0f) + xy;
		data[5].Pos = glm::vec2(-1.0f * wh.x,  1.0f) + xy;
	}
	else if (state.Anchor == GUIAnchor_BottomRight)
	{
		wh = wh * 2.0f - 1.0f;
		data[0].Pos = glm::vec2(-1.0f * wh.x, -1.0f * wh.y) + xy;
		data[1].Pos = glm::vec2( 1.0f		, -1.0f * wh.y) + xy;
		data[2].Pos = glm::vec2(-1.0f * wh.x,  1.0f) + xy;
		data[3].Pos = glm::vec2( 1.0f		, -1.0f * wh.y) + xy;
		data[4].Pos = glm::vec2( 1.0f		,  1.0f) + xy;
		data[5].Pos = glm::vec2(-1.0f * wh.x,  1.0f) + xy;
	}
	return data;
}
