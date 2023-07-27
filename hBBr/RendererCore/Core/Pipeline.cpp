#include "Pipeline.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
Pipeline::Pipeline()
{

}

Pipeline::~Pipeline()
{
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout);
	if (_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(VulkanManager::GetManager()->GetDevice(), _pipeline, VK_NULL_HANDLE);
		_pipeline = VK_NULL_HANDLE;
	}
}

void Pipeline::CreatePipelineObject(VkRenderPass renderPass, uint32_t subpassCount, PipelineType pipelineType)
{
	if (subpassCount <= 0)
	{
		MessageOut("Create Pipeline Object Error,subpass count is 0.", true, true);
	}
	_pipelineType = pipelineType;
	if (_pipelineType == PipelineType::Graphics)
	{
		for (int i = 0; i < (int)subpassCount; i++)
		{
			BuildGraphicsPipelineState(renderPass, subpassCount - 1);
		}
	}
}

void Pipeline::SetColorBlend(bool bEnable, StaticBlendState blendState)
{
	_graphicsPipelineCreateInfoCache.colorBlendAttachmentStates.clear();
	VkPipelineColorBlendAttachmentState attachment = {};
	attachment.blendEnable = bEnable ? VK_TRUE : VK_FALSE;
	//公式：<srcFactor(*new color)><BlendOp><dstFactor(*old color)>
	attachment.colorBlendOp = (VkBlendOp)blendState.color_op;
	attachment.srcColorBlendFactor = (VkBlendFactor)blendState.color_src_factor;
	attachment.dstColorBlendFactor = (VkBlendFactor)blendState.color_dest_factor;
	attachment.alphaBlendOp = (VkBlendOp)blendState.alpha_op;
	attachment.srcAlphaBlendFactor = (VkBlendFactor)blendState.alpha_src_factor;
	attachment.dstAlphaBlendFactor = (VkBlendFactor)blendState.alpha_dest_factor;
	attachment.colorWriteMask = (VkColorComponentFlagBits)blendState.colorWriteMask;
	_graphicsPipelineCreateInfoCache.colorBlendAttachmentStates.resize(blendState.outputAttachmentCount);
	for (auto i = 0; i < _graphicsPipelineCreateInfoCache.colorBlendAttachmentStates.size(); i++)
	{
		_graphicsPipelineCreateInfoCache.colorBlendAttachmentStates[i] = attachment;
	}
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.attachmentCount = blendState.outputAttachmentCount;
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.pAttachments = _graphicsPipelineCreateInfoCache.colorBlendAttachmentStates.data();
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.logicOpEnable = VK_FALSE;
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.logicOp = VK_LOGIC_OP_NO_OP;
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.blendConstants[0] = 0.0f;
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.blendConstants[1] = 0.0f;
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.blendConstants[2] = 0.0f;
	_graphicsPipelineCreateInfoCache.ColorBlendInfo.blendConstants[3] = 0.0f;
	_graphicsPipelineCreateInfoCache.CreateInfo.pColorBlendState = &_graphicsPipelineCreateInfoCache.ColorBlendInfo;
}


void Pipeline::SetRenderRasterizer(Rasterizer rasterizer)
{
	_graphicsPipelineCreateInfoCache.rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.rasterInfo.depthClampEnable = VK_FALSE;
	_graphicsPipelineCreateInfoCache.rasterInfo.rasterizerDiscardEnable = VK_FALSE;
	_graphicsPipelineCreateInfoCache.rasterInfo.polygonMode = (VkPolygonMode)rasterizer.polygonMode;
	_graphicsPipelineCreateInfoCache.rasterInfo.lineWidth = 1.0f;
	_graphicsPipelineCreateInfoCache.rasterInfo.cullMode = (VkCullModeFlagBits)rasterizer.cullMode;
	_graphicsPipelineCreateInfoCache.rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	_graphicsPipelineCreateInfoCache.rasterInfo.depthBiasEnable = VK_FALSE;
	//
	_graphicsPipelineCreateInfoCache.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
	_graphicsPipelineCreateInfoCache.inputAssemblyInfo.topology = (VkPrimitiveTopology)rasterizer.primitiveTopology;
	//
	_graphicsPipelineCreateInfoCache.CreateInfo.pRasterizationState = &_graphicsPipelineCreateInfoCache.rasterInfo;
	_graphicsPipelineCreateInfoCache.CreateInfo.pInputAssemblyState = &_graphicsPipelineCreateInfoCache.inputAssemblyInfo;
}

void Pipeline::SetRenderDepthStencil(DepthStencil depthStencil)
{
	_graphicsPipelineCreateInfoCache.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.minDepthBounds = 0;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.maxDepthBounds = 1;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.depthWriteEnable = depthStencil.bEnableDepth ? VK_TRUE : VK_FALSE;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.depthTestEnable = depthStencil.bEnableDepthTest ? VK_TRUE : VK_FALSE;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.depthCompareOp = (VkCompareOp)depthStencil.depthCompareOp;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.stencilTestEnable = depthStencil.bEnableStencilTest ? VK_TRUE : VK_FALSE;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.front.compareOp = (VkCompareOp)depthStencil.stencilCompareOp;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.front.failOp = (VkStencilOp)depthStencil.stencilTestFailOp;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.front.passOp = (VkStencilOp)depthStencil.stencilTestPassOp;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.front.depthFailOp = (VkStencilOp)depthStencil.stencilDepthFailOp;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.front.compareMask = 0xff;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.front.writeMask = 0xff;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.front.reference = depthStencil.reference;
	_graphicsPipelineCreateInfoCache.depthStencilInfo.back = _graphicsPipelineCreateInfoCache.depthStencilInfo.front;
	_graphicsPipelineCreateInfoCache.CreateInfo.pDepthStencilState = &_graphicsPipelineCreateInfoCache.depthStencilInfo;
}

void Pipeline::SetVertexInput(VertexInputLayout vertexInputLayout)
{
	_graphicsPipelineCreateInfoCache.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.vertexInputBindingDescs.resize(1);
	_graphicsPipelineCreateInfoCache.vertexInputBindingDescs[0] = {};
	_graphicsPipelineCreateInfoCache.vertexInputBindingDescs[0].binding = 0;
	_graphicsPipelineCreateInfoCache.vertexInputBindingDescs[0].stride = vertexInputLayout.inputSize;
	_graphicsPipelineCreateInfoCache.vertexInputBindingDescs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	_graphicsPipelineCreateInfoCache.vertexInputAttributes.resize(vertexInputLayout.inputLayouts.size());
	uint32_t offset = 0;
	for (int i = 0; i < _graphicsPipelineCreateInfoCache.vertexInputAttributes.size(); i++)
	{
		_graphicsPipelineCreateInfoCache.vertexInputAttributes[i] = {};
		_graphicsPipelineCreateInfoCache.vertexInputAttributes[i].format = vertexInputLayout.inputLayouts[i];
		_graphicsPipelineCreateInfoCache.vertexInputAttributes[i].location = i;
		_graphicsPipelineCreateInfoCache.vertexInputAttributes[i].binding = 0;
		_graphicsPipelineCreateInfoCache.vertexInputAttributes[i].offset = offset;
		if (vertexInputLayout.inputLayouts[i] == VK_FORMAT_R32G32B32A32_SFLOAT)
			offset += 4 * sizeof(float);
		else if (vertexInputLayout.inputLayouts[i] == VK_FORMAT_R32G32B32_SFLOAT)
			offset += 3 * sizeof(float);
		else if (vertexInputLayout.inputLayouts[i] == VK_FORMAT_R32G32_SFLOAT)
			offset += 2 * sizeof(float);
		else if (vertexInputLayout.inputLayouts[i] == VK_FORMAT_R32_SFLOAT)
			offset += 1 * sizeof(float);
	}
	_graphicsPipelineCreateInfoCache.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(_graphicsPipelineCreateInfoCache.vertexInputAttributes.size());
	_graphicsPipelineCreateInfoCache.vertexInputInfo.pVertexAttributeDescriptions = _graphicsPipelineCreateInfoCache.vertexInputAttributes.data();// Optional
	_graphicsPipelineCreateInfoCache.vertexInputInfo.vertexBindingDescriptionCount = 1;
	_graphicsPipelineCreateInfoCache.vertexInputInfo.pVertexBindingDescriptions = _graphicsPipelineCreateInfoCache.vertexInputBindingDescs.data();// Optional

	_graphicsPipelineCreateInfoCache.CreateInfo.pVertexInputState = &_graphicsPipelineCreateInfoCache.vertexInputInfo;
}

void Pipeline::SetDepthStencil()
{

}

void Pipeline::SetPipelineLayout(std::vector<VkDescriptorSetLayout> layout)
{
	VulkanManager::GetManager()->CreatePipelineLayout(layout, _pipelineLayout);
	_graphicsPipelineCreateInfoCache.CreateInfo.layout = _pipelineLayout;
}

void Pipeline::SetVertexShaderAndPixelShader(ShaderCache vs, ShaderCache ps)
{
	//set shader
	//_graphicsPipelineCreateInfoCache.CreateInfo.stageCount = _countof(shader_stage);
	//_graphicsPipelineCreateInfoCache.CreateInfo.pStages = shader_stage;
	static VkPipelineShaderStageCreateInfo info[] = {
		vs.shaderStageInfo, 
		ps.shaderStageInfo
	};
	_graphicsPipelineCreateInfoCache.CreateInfo.stageCount = 2;
	_graphicsPipelineCreateInfoCache.CreateInfo.pStages = info;

}

void Pipeline::BuildGraphicsPipelineState(VkRenderPass renderPass, uint32_t subpassIndex)
{
	//-----------------------------------------------------------------------------------Dynamic State
	_graphicsPipelineCreateInfoCache.dynamicStates.clear();
	_graphicsPipelineCreateInfoCache.dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
	_graphicsPipelineCreateInfoCache.dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
	_graphicsPipelineCreateInfoCache.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.dynamicStateInfo.pNext = NULL;
	_graphicsPipelineCreateInfoCache.dynamicStateInfo.pDynamicStates = _graphicsPipelineCreateInfoCache.dynamicStates.data();
	_graphicsPipelineCreateInfoCache.dynamicStateInfo.dynamicStateCount = (uint32_t)_graphicsPipelineCreateInfoCache.dynamicStates.size();
	//-----------------------------------------------------------------------------------Multisampling 多重采样,不使用
	_graphicsPipelineCreateInfoCache.msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//msCreateInfo.rasterizationSamples = (MSAASamples != 0) ? MSAASamples : VK_SAMPLE_COUNT_1_BIT;
	_graphicsPipelineCreateInfoCache.msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	_graphicsPipelineCreateInfoCache.msInfo.sampleShadingEnable = VK_FALSE;
	//-----------------------------------------------------------------------------------Viewport
	_graphicsPipelineCreateInfoCache.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.viewportInfo.viewportCount = 1;
	_graphicsPipelineCreateInfoCache.viewportInfo.scissorCount = 1;
	//-----------------------------------------------------------------------------------VertexInput
	//-----------------------------------------------------------------------------------InputAssemblyState
	_graphicsPipelineCreateInfoCache.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
	_graphicsPipelineCreateInfoCache.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//-----------------------------------------------------------------------------------RasterizationState
	_graphicsPipelineCreateInfoCache.rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
	_graphicsPipelineCreateInfoCache.rasterInfo.cullMode = VK_CULL_MODE_NONE;
	_graphicsPipelineCreateInfoCache.rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	_graphicsPipelineCreateInfoCache.rasterInfo.lineWidth = 1.0f;
	//-----------------------------------------------------------------------------------ColorBlendState
	//-----------------------------------------------------------------------------------DepthStencilState
	VkPipelineDepthStencilStateCreateInfo depthInfo = {};
	depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	// 
	//set shader
	//info.stageCount = _countof(shader_stage);
	//info.pStages = shader_stage;
	//
	_graphicsPipelineCreateInfoCache.CreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	_graphicsPipelineCreateInfoCache.CreateInfo.flags = 0;
	_graphicsPipelineCreateInfoCache.CreateInfo.pViewportState = &_graphicsPipelineCreateInfoCache.viewportInfo;
	_graphicsPipelineCreateInfoCache.CreateInfo.pMultisampleState = &_graphicsPipelineCreateInfoCache.msInfo;
	//_graphicsPipelineCreateInfoCache.CreateInfo.pDepthStencilState = &depthInfo;
	_graphicsPipelineCreateInfoCache.CreateInfo.pDynamicState = &_graphicsPipelineCreateInfoCache.dynamicStateInfo;
	//_graphicsPipelineCreateInfoCache.CreateInfo.layout = bd->PipelineLayout;
	_graphicsPipelineCreateInfoCache.CreateInfo.renderPass = renderPass;
	_graphicsPipelineCreateInfoCache.CreateInfo.subpass = subpassIndex;

	VulkanManager::GetManager()->CreateGraphicsPipeline(_graphicsPipelineCreateInfoCache.CreateInfo, _pipeline);
}
