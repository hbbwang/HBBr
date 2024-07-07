#include "Pipeline.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
std::map<PipelineIndex, std::unique_ptr<PipelineObject>> PipelineManager::_graphicsPipelines;
std::map<PipelineIndex, std::unique_ptr<PipelineObject>> PipelineManager::_computePipelines;

PipelineObject::~PipelineObject()
{
	if (pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(VulkanManager::GetManager()->GetDevice(), pipeline, VK_NULL_HANDLE);
		pipeline = VK_NULL_HANDLE;
	}
}

PipelineManager::PipelineManager()
{

}

PipelineManager::~PipelineManager()
{

}

PipelineObject* PipelineManager::CreatePipelineObject(VkGraphicsPipelineCreateInfoCache& createInfo, VkPipelineLayout layout,  VkRenderPass renderPass, PipelineIndex pipelineIndex, uint32_t subpassCount, PipelineType pipelineType)
{
	if (subpassCount <= 0)
	{
		MessageOut("Create Pipeline Object Error,subpass count is 0.", true, true);
	}
	//createInfo.graphicsName = pipelineName;
	std::unique_ptr<PipelineObject> newPSO = std::make_unique<PipelineObject>();
	newPSO->pipelineType = pipelineType;
	newPSO->layout = layout;
	newPSO->bHasMaterialParameterVS = createInfo.bHasMaterialParameterVS;
	newPSO->bHasMaterialParameterPS = createInfo.bHasMaterialParameterPS;
	newPSO->bHasMaterialTexture = createInfo.bHasMaterialTexture;
	SetPipelineLayout(createInfo, layout);
	if (pipelineType == PipelineType::Graphics)
	{
		for (int i = 0; i < (int)subpassCount; i++)
		{
			BuildGraphicsPipelineState(createInfo, renderPass, subpassCount - 1, newPSO->pipeline);
		}
		PipelineObject* result = newPSO.get();
		_graphicsPipelines.emplace(std::make_pair(pipelineIndex, std::move(newPSO)));
		return result;
	}
	return nullptr;
}

PipelineObject* PipelineManager::GetGraphicsPipelineMap(PipelineIndex index)
{
	auto it = _graphicsPipelines.find(index);
	if (it != _graphicsPipelines.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void PipelineManager::ClearPipelineObjects()
{
	_graphicsPipelines.clear();
	_computePipelines.clear();
}

void PipelineManager::RemovePipelineObjects(PipelineIndex& index)
{
	{
		auto it = _graphicsPipelines.find(index);
		if (it != _graphicsPipelines.end())
		{
			_graphicsPipelines.erase(index);
		}
	}
	{
		auto it = _computePipelines.find(index);
		if (it != _computePipelines.end())
		{
			_computePipelines.erase(index);
		}
	}
}

void PipelineManager::SetColorBlend(VkGraphicsPipelineCreateInfoCache& createInfo, bool bEnable, StaticBlendState blendState)
{
	createInfo.colorBlendAttachmentStates.clear();
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
	createInfo.colorBlendAttachmentStates.resize(blendState.outputAttachmentCount);
	for (auto i = 0; i < createInfo.colorBlendAttachmentStates.size(); i++)
	{
		createInfo.colorBlendAttachmentStates[i] = attachment;
	}
	createInfo.ColorBlendInfo.attachmentCount = blendState.outputAttachmentCount;
	createInfo.ColorBlendInfo.pAttachments = createInfo.colorBlendAttachmentStates.data();
	createInfo.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	createInfo.ColorBlendInfo.logicOpEnable = VK_FALSE;
	createInfo.ColorBlendInfo.logicOp = VK_LOGIC_OP_NO_OP;
	createInfo.ColorBlendInfo.blendConstants[0] = 0.0f;
	createInfo.ColorBlendInfo.blendConstants[1] = 0.0f;
	createInfo.ColorBlendInfo.blendConstants[2] = 0.0f;
	createInfo.ColorBlendInfo.blendConstants[3] = 0.0f;
	createInfo.CreateInfo.pColorBlendState = &createInfo.ColorBlendInfo;
}


void PipelineManager::SetRenderRasterizer(VkGraphicsPipelineCreateInfoCache& createInfo, Rasterizer rasterizer)
{
	createInfo.rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	createInfo.rasterInfo.depthClampEnable = VK_FALSE;
	createInfo.rasterInfo.rasterizerDiscardEnable = VK_FALSE;
	createInfo.rasterInfo.polygonMode = (VkPolygonMode)rasterizer.polygonMode;
	createInfo.rasterInfo.lineWidth = 1.0f;
	createInfo.rasterInfo.cullMode = (VkCullModeFlagBits)rasterizer.cullMode;
	createInfo.rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	createInfo.rasterInfo.depthBiasEnable = VK_FALSE;
	//
	createInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	createInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
	createInfo.inputAssemblyInfo.topology = (VkPrimitiveTopology)rasterizer.primitiveTopology;
	//
	createInfo.CreateInfo.pRasterizationState = &createInfo.rasterInfo;
	createInfo.CreateInfo.pInputAssemblyState = &createInfo.inputAssemblyInfo;
}

void PipelineManager::SetRenderDepthStencil(VkGraphicsPipelineCreateInfoCache& createInfo, DepthStencil depthStencil)
{
	createInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	createInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	createInfo.depthStencilInfo.minDepthBounds = 0;
	createInfo.depthStencilInfo.maxDepthBounds = 1;
	createInfo.depthStencilInfo.depthWriteEnable = depthStencil.bEnableDepth ? VK_TRUE : VK_FALSE;
	createInfo.depthStencilInfo.depthTestEnable = depthStencil.bEnableDepthTest ? VK_TRUE : VK_FALSE;
	createInfo.depthStencilInfo.depthCompareOp = (VkCompareOp)depthStencil.depthCompareOp;
	createInfo.depthStencilInfo.stencilTestEnable = depthStencil.bEnableStencilTest ? VK_TRUE : VK_FALSE;
	createInfo.depthStencilInfo.front.compareOp = (VkCompareOp)depthStencil.stencilCompareOp;
	createInfo.depthStencilInfo.front.failOp = (VkStencilOp)depthStencil.stencilTestFailOp;
	createInfo.depthStencilInfo.front.passOp = (VkStencilOp)depthStencil.stencilTestPassOp;
	createInfo.depthStencilInfo.front.depthFailOp = (VkStencilOp)depthStencil.stencilDepthFailOp;
	createInfo.depthStencilInfo.front.compareMask = 0xff;
	createInfo.depthStencilInfo.front.writeMask = 0xff;
	createInfo.depthStencilInfo.front.reference = depthStencil.reference;
	createInfo.depthStencilInfo.back = createInfo.depthStencilInfo.front;
	createInfo.CreateInfo.pDepthStencilState = &createInfo.depthStencilInfo;
}

void PipelineManager::SetVertexInput(VkGraphicsPipelineCreateInfoCache& createInfo, VertexInputLayout vertexInputLayout)
{
	SetVertexInput(createInfo, vertexInputLayout.inputSize, vertexInputLayout.inputRate, vertexInputLayout.inputLayouts);
}

void PipelineManager::SetVertexInput(VkGraphicsPipelineCreateInfoCache& createInfo, uint32_t vertexInputStride, VkVertexInputRate vertexInputRate, std::vector<VkFormat>inputLayout)
{
	createInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	createInfo.vertexInputBindingDescs.resize(1);
	createInfo.vertexInputBindingDescs[0] = {};
	createInfo.vertexInputBindingDescs[0].binding = 0;
	createInfo.vertexInputBindingDescs[0].stride = vertexInputStride;
	createInfo.vertexInputBindingDescs[0].inputRate = vertexInputRate;
	createInfo.vertexInputAttributes.resize(inputLayout.size());
	uint32_t offset = 0;
	for (int i = 0; i < createInfo.vertexInputAttributes.size(); i++)
	{
		createInfo.vertexInputAttributes[i] = {};
		createInfo.vertexInputAttributes[i].format = inputLayout[i];
		createInfo.vertexInputAttributes[i].location = i;
		createInfo.vertexInputAttributes[i].binding = 0;
		createInfo.vertexInputAttributes[i].offset = offset;
		if (inputLayout[i] == VK_FORMAT_R32G32B32A32_SFLOAT)
			offset += 4 * sizeof(float);
		else if (inputLayout[i] == VK_FORMAT_R32G32B32_SFLOAT)
			offset += 3 * sizeof(float);
		else if (inputLayout[i] == VK_FORMAT_R32G32_SFLOAT)
			offset += 2 * sizeof(float);
		else if (inputLayout[i] == VK_FORMAT_R32_SFLOAT)
			offset += 1 * sizeof(float);
	}
	createInfo.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(createInfo.vertexInputAttributes.size());
	createInfo.vertexInputInfo.pVertexAttributeDescriptions = createInfo.vertexInputAttributes.data();// Optional
	createInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;
	createInfo.vertexInputInfo.pVertexBindingDescriptions = createInfo.vertexInputBindingDescs.data();// Optional
	createInfo.CreateInfo.pVertexInputState = &createInfo.vertexInputInfo;
}

void PipelineManager::SetDepthStencil(VkGraphicsPipelineCreateInfoCache& createInfo)
{

}

void PipelineManager::SetPipelineLayout(VkGraphicsPipelineCreateInfoCache& createInfo, VkPipelineLayout pipelineLayout)
{
	createInfo.CreateInfo.layout = pipelineLayout;
}

void PipelineManager::SetVertexShaderAndPixelShader(VkGraphicsPipelineCreateInfoCache& createInfo, ShaderCache *vs, ShaderCache *ps)
{
	//vs.shaderStageInfo.module = vs.shaderModule[varient];
	//ps.shaderStageInfo.module = ps.shaderModule[varient];

	createInfo.stages.push_back(vs->shaderStageInfo);
	createInfo.stages.push_back(ps->shaderStageInfo);

	createInfo.CreateInfo.stageCount = (uint32_t)createInfo.stages.size();
	createInfo.CreateInfo.pStages = createInfo.stages.data();

	createInfo.bHasMaterialParameterVS = vs->params.size() > 0;
	createInfo.bHasMaterialParameterPS = ps->params.size() > 0;
	createInfo.bHasMaterialTexture = vs->texs.size() > 0 && ps->texs.size() > 0;
}

void PipelineManager::BuildGraphicsPipelineState(VkGraphicsPipelineCreateInfoCache& createInfo, VkRenderPass renderPass, uint32_t subpassIndex, VkPipeline& pipelineObj)
{
	//-----------------------------------------------------------------------------------Dynamic State
	createInfo.dynamicStates.clear();
	createInfo.dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
	createInfo.dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
	createInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	createInfo.dynamicStateInfo.pNext = nullptr;
	createInfo.dynamicStateInfo.pDynamicStates = createInfo.dynamicStates.data();
	createInfo.dynamicStateInfo.dynamicStateCount = (uint32_t)createInfo.dynamicStates.size();
	//-----------------------------------------------------------------------------------Multisampling 多重采样,不使用
	createInfo.msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//msCreateInfo.rasterizationSamples = (MSAASamples != 0) ? MSAASamples : VK_SAMPLE_COUNT_1_BIT;
	createInfo.msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.msInfo.sampleShadingEnable = VK_FALSE;
	//-----------------------------------------------------------------------------------Viewport
	createInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	createInfo.viewportInfo.viewportCount = 1;
	createInfo.viewportInfo.scissorCount = 1;
	//-----------------------------------------------------------------------------------VertexInput
	//-----------------------------------------------------------------------------------InputAssemblyState
	//-----------------------------------------------------------------------------------RasterizationState
	//-----------------------------------------------------------------------------------ColorBlendState
	//-----------------------------------------------------------------------------------DepthStencilState
	VkPipelineDepthStencilStateCreateInfo depthInfo = {};
	depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	//
	createInfo.CreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.CreateInfo.flags = 0;
	createInfo.CreateInfo.pViewportState = &createInfo.viewportInfo;
	createInfo.CreateInfo.pMultisampleState = &createInfo.msInfo;
	//createInfo.CreateInfo.pDepthStencilState = &depthInfo;
	createInfo.CreateInfo.pDynamicState = &createInfo.dynamicStateInfo;
	//createInfo.CreateInfo.layout = bd->PipelineLayout;
	createInfo.CreateInfo.renderPass = renderPass;
	createInfo.CreateInfo.subpass = subpassIndex;

	VulkanManager::GetManager()->CreateGraphicsPipeline(createInfo.CreateInfo, pipelineObj);
}

void PipelineManager::ClearCreateInfo(VkGraphicsPipelineCreateInfoCache& createInfo)
{
	createInfo = VkGraphicsPipelineCreateInfoCache();
}

PipelineIndex PipelineManager::AddPipelineObject(std::weak_ptr<ShaderCache>vs, std::weak_ptr<ShaderCache>ps, VkPipeline pipeline, VkPipelineLayout pipelineLayout)
{
	PipelineIndex index = PipelineIndex::GetPipelineIndex(vs,ps);
	std::unique_ptr<PipelineObject> newPSO = std::make_unique<PipelineObject>();
	newPSO->pipeline = pipeline;
	newPSO->bHasMaterialTexture = vs.lock()->texs.size() > 0;
	newPSO->bHasMaterialParameterVS = vs.lock()->params.size() > 0;
	newPSO->bHasMaterialParameterPS = ps.lock()->params.size() > 0;
	newPSO->layout = pipelineLayout;
	newPSO->pipelineType = PipelineType::Graphics;
	_graphicsPipelines.emplace(std::make_pair(index, std::move(newPSO)));
	return index;
}
