#include "GUIPass.h"
#include "VulkanRenderer.h"

GUIPass::~GUIPass()
{
	const auto& manager = VulkanManager::GetManager();
	_vertexBuffer.reset();
	manager->DestroyPipelineLayout(_pipelineLayout);
	for (auto i : _guiPipelines)
	{
		manager->DestroyPipeline(i.second);
	}
	_drawList.clear();
	_guiPipelines.clear();
	manager->DestroyDescriptorSetLayout(_texDescriptorSetLayout);
	manager->DestroyDescriptorSetLayout(_ubDescriptorSetLayout);
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
	manager->CreateDescripotrSetLayout({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC }, _ubDescriptorSetLayout, VK_SHADER_STAGE_FRAGMENT_BIT);
	manager->CreateDescripotrSetLayout({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, _texDescriptorSetLayout, VK_SHADER_STAGE_FRAGMENT_BIT);
	_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	manager->CreatePipelineLayout(
		{
			_ubDescriptorSetLayout,
			_texDescriptorSetLayout,
		}
	, _pipelineLayout);
	//Set Pass Name
	_passName = "GUI Render Pass";

	//Create GUIShader Pipeline
	CreatePipeline("Base", "GUIShader");
}

void GUIPass::ShowPerformance()
{
	GUIDrawText("ShowPerf_frame",
		HString("Frame " + HString::FromFloat(_renderer->GetFrameRate(), 2) + " ms\n").c_wstr()
		, 0, 0, 200, 200, GUIDrawState(GUIAnchor_TopLeft, false, glm::vec4(1)), 20.0f);
	GUIDrawText("ShowPerf_fps",
		HString("FPS " + HString::FromUInt((uint32_t)(1.0f / (float)(_renderer->GetFrameRate() / 1000.0)))).c_wstr()
		, 0, 20.0f, 200, 200, GUIDrawState(GUIAnchor_TopLeft, false, glm::vec4(1)), 20.0f);
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
	GUIDrawImage("TestImage", Texture::GetSystemTexture("TestTex"), 0, 0, 200, 200, GUIDrawState(GUIAnchor_TopLeft, false, glm::vec4(1, 1, 1, 0.95)));
	ShowPerformance();

	GUIDrawText("fonttest",
		L"ABCD测试你好123我靠你妈的。 "
		, 0, 20.0f, 200, 200, GUIDrawState(GUIAnchor_CenterCenter, false, glm::vec4(1)), 20.0f);

	//收集顶点数据一次性使用
	std::vector<GUIVertexData> vertices;
	for (auto i : _drawList)
	{
		//vertex buffer
		vertices.insert(vertices.end(), i.second.Data.begin(), i.second.Data.end());
		//textures
		i.second.tex_descriptorSet->UpdateTextureDescriptorSet({ i.second.BaseTexture });
		uint32_t ubSize = manager->GetMinUboAlignmentSize(sizeof(GUIUniformBuffer));
		uint32_t ubOffset = 0;
		i.second.ub_descriptorSet->ResizeDescriptorBuffer(ubSize * (i.second.States.size()));
		i.second.ub_descriptorSet->UpdateDescriptorSet(ubSize * (i.second.States.size()));
		for (auto s : i.second.States)
		{
			//uniform buffers
			i.second.ub_descriptorSet->BufferMapping(&s.uniformBuffer, ubOffset, ubSize);
			ubOffset += ubSize;
		}
	}

	//vertex buffer
	_vertexBuffer->BufferMapping(vertices.data(), 0, sizeof(GUIVertexData) * vertices.size());
	VkDeviceSize vbOffset = 0;
	VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
	vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);

	VkPipeline pipeline = VK_NULL_HANDLE;
	for (auto i : _drawList)
	{
		VkPipeline currentPipeline = _guiPipelines[i.second.PipelineTag];
		if (currentPipeline != pipeline)
		{
			pipeline = currentPipeline;
			manager->CmdCmdBindPipeline(cmdBuf, pipeline);
		}
		//textures
		vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &i.second.tex_descriptorSet->GetDescriptorSet(), 0, 0);
		//vkCmdSetScissor(cmdBuf, 0, 1, &i.second.viewport);
		uint32_t ubOffset = 0;
		for (auto s : i.second.States)
		{
			//uniform buffers
			vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &i.second.ub_descriptorSet->GetDescriptorSet(), 1, &ubOffset);
			//draw primitive
			vkCmdDraw(cmdBuf, 6, 1, vbOffset, 0);
			ubOffset += manager->GetMinUboAlignmentSize(sizeof(GUIUniformBuffer));
			vbOffset += 6;
		}
	}
	//End...
	EndRenderPass();
}

void GUIPass::PassReset()
{
}

void GUIPass::GUIDrawText(HString tag, const wchar_t* text, float x, float y, float w, float h, GUIDrawState state , float fontSize)
{
	auto textLength = wcslen(text);
	if (textLength <= 0)
	{
		return;
	}
	state.uniformBuffer.Color = glm::vec4(1,1,1,2);
	state.uniformBuffer.Flags = IsFont;
	GUIPrimitive* prim = GetPrimitve(tag, state, textLength, "Base", x, y, w, h);
	std::vector<GUIVertexData>textMeshes;
	if (prim->BaseTexture != Texture::GetFontTexture())
	{
		prim->BaseTexture = Texture::GetFontTexture();
		prim->BaseTexture->SetSampler(TextureSampler_Linear_Clamp);
		prim->tex_descriptorSet->NeedUpdate();
	}
	//计算每个文字面片位置
	float tx = 0;
	float ty = 0;
	prim->fontCharacter.resize(textLength);
	for (int i = 0; i < textLength; i++)
	{
		auto textChar = text[i];
		if (textChar == L'\n')//下一行
		{
			tx = 0;
			ty += fontSize;
		}
		prim->fontCharacter[i] = textChar;
		//文字不存在fixed模式,文字的大小不应该被变形
		prim->States[i].bFixed = false;
		//获取文字信息
		auto info = Texture::GetFontInfo(prim->fontCharacter[i]);
		prim->States[i].uniformBuffer.UVSetting = glm::vec4(info->posX, info->posY, info->sizeX, info->sizeY);
		prim->States[i].uniformBuffer.TextureSize = Texture::GetFontTexture()->GetImageSize().width;
		//文字像素大小
		if (textChar == L' ')
		{
			prim->States[i].Scale.x = fontSize / 2.0f;
		}
		else
		{
			prim->States[i].Scale = glm::vec2(info->sizeX / info->sizeY, 1) * fontSize;
		}
		//文字按顺序偏移 
		prim->States[i].Translate = glm::vec2(tx, ty );
		auto newTextMesh = GetGUIPanel(prim->States[i], x, y, 1, 1);
		textMeshes.insert(textMeshes.end(), newTextMesh.begin(), newTextMesh.end());
		tx += prim->States[i].Scale.x + 1;
	}
	prim->Data = textMeshes;
}

void GUIPass::GUIDrawImage(HString tag, Texture* texture, float x, float y, float w, float h, GUIDrawState state)
{
	GUIPrimitive* prim = GetPrimitve(tag, state, 1, "Base", x, y, w, h);
	prim->Data = GetGUIPanel(state, x, y, w, h);
	prim->States[0].uniformBuffer.UVSetting = glm::vec4(0, 0, 1, 1);
	prim->States[0].uniformBuffer.TextureSize = 1.0f;
	if (prim->BaseTexture != texture)
	{
		prim->BaseTexture = texture;
		prim->tex_descriptorSet->NeedUpdate();
	}
}

std::vector<GUIVertexData> GUIPass::GetGUIPanel(GUIDrawState& state, float x, float y, float w, float h)
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
	glm::vec2 xy = state.Translate + glm::vec2(x, y);
	glm::vec2 wh = state.Scale * glm::vec2(w, h);
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
		data[0].Pos = glm::vec2(-1.0f		, -1.0f) + xy;
		data[1].Pos = glm::vec2(1.0f * wh.x	, -1.0f) + xy;
		data[2].Pos = glm::vec2(-1.0f		, 1.0f * wh.y) + xy;
		data[3].Pos = glm::vec2(1.0f * wh.x	, -1.0f) + xy;
		data[4].Pos = glm::vec2(1.0f * wh.x	, 1.0f * wh.y) + xy;
		data[5].Pos = glm::vec2(-1.0f		, 1.0f * wh.y) + xy;
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

GUIPrimitive* GUIPass::GetPrimitve(HString& tag, GUIDrawState& state, int stateCount, HString pipelineTag, float x, float y, float w, float h)
{
	auto dit = _drawList.find(tag);
	GUIPrimitive* prim = NULL;
	if (dit != _drawList.end())
	{
		prim = &dit->second;
	}
	else
	{
		_drawList.emplace(tag, GUIPrimitive());
		prim = &_drawList[tag];
		prim->PipelineTag = pipelineTag;
	}

	prim->States.resize(stateCount);
	for (int i = 0; i < stateCount; i++)
	{
		prim->States[i] = state;
	}

	if (!prim->ub_descriptorSet)
	{
		prim->ub_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _ubDescriptorSetLayout, stateCount, BufferSizeRange, VK_SHADER_STAGE_FRAGMENT_BIT));
	}
	if (!prim->tex_descriptorSet)
	{
		prim->tex_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _texDescriptorSetLayout, 1, 0, VK_SHADER_STAGE_FRAGMENT_BIT));
	}
	prim->viewport = { (int)x,(int)y,(uint32_t)w,(uint32_t)h };
	return prim;
}

void GUIPass::CreatePipeline(HString pipelineTag, HString shaderName)
{
	//CraetePipeline..
	VkPipeline pipeline = VK_NULL_HANDLE;
	auto vsCache = Shader::_vsShader[shaderName];
	auto psCache = Shader::_psShader[shaderName];
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
			1,
			CW_RGBA,
			BO_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA,//color
			BO_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA //alpha
		));
	PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
	//PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
	PipelineManager::SetVertexInput(pipelineCreateInfo, vertexInputLayout);
	PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache, psCache);
	PipelineManager::SetPipelineLayout(pipelineCreateInfo, _pipelineLayout);
	PipelineManager::BuildGraphicsPipelineState(pipelineCreateInfo, _renderPass, 0, pipeline);
	_guiPipelines.emplace(std::make_pair(pipelineTag, pipeline));
}
