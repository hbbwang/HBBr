#pragma once
//#include "Common.h"
#include <vector>
#include "VulkanManager.h"
#include "Shader.h"
#include "Pass/PassType.h"
struct VkGraphicsPipelineCreateInfoCache
{
	HString graphicsName;//ps or cs name
	//------------------------State
	VkGraphicsPipelineCreateInfo					CreateInfo{};
	//------------------------Rasterizer
	VkPipelineRasterizationStateCreateInfo			rasterInfo{};
	//------------------------Blend
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
	VkPipelineColorBlendStateCreateInfo				ColorBlendInfo{};
	//------------------------Dynamic State
	std::vector<VkDynamicState>						dynamicStates;
	VkPipelineDynamicStateCreateInfo				dynamicStateInfo = {};
	//------------------------ViewPort
	VkPipelineViewportStateCreateInfo				viewportInfo{};
	//------------------------VertexInput
	std::vector<VkVertexInputBindingDescription>	vertexInputBindingDescs;
	std::vector<VkVertexInputAttributeDescription>	vertexInputAttributes;
	VkPipelineVertexInputStateCreateInfo			vertexInputInfo{};
	//------------------------Input Assembly
	VkPipelineInputAssemblyStateCreateInfo			inputAssemblyInfo{};
	//------------------------MultiSampling
	VkPipelineMultisampleStateCreateInfo			msInfo{};
	//------------------------Depth Stencil
	VkPipelineDepthStencilStateCreateInfo			depthStencilInfo{};
	//Stages
	std::vector<VkPipelineShaderStageCreateInfo>	stages;

	bool bHasMaterialParameter = false;

	bool bHasMaterialTexture = false;
	
};

enum ColorWriteMask
{
	CW_R = VK_COLOR_COMPONENT_R_BIT,
	CW_G = VK_COLOR_COMPONENT_G_BIT,
	CW_B = VK_COLOR_COMPONENT_B_BIT,
	CW_A = VK_COLOR_COMPONENT_A_BIT,
	CW_RG = CW_R | CW_G,
	CW_GB = CW_G | CW_B,
	CW_RB = CW_R | CW_B,
	CW_RGB = CW_R | CW_G | CW_B,
	CW_RGBA = CW_R | CW_G | CW_B | CW_A,
	CW_RA = CW_R | CW_A,
};

enum BlendOperation
{
	BO_ADD				= VK_BLEND_OP_ADD,
	BO_SUBTRACT			= VK_BLEND_OP_SUBTRACT,
	BO_REVERSE_SUBTRACT = VK_BLEND_OP_REVERSE_SUBTRACT,
	BO_MIN				= VK_BLEND_OP_MIN,
	BO_MAX				= VK_BLEND_OP_MAX,
};

enum BlendFactor
{
	BF_ZERO = VK_BLEND_FACTOR_ZERO,
	BF_ONE = VK_BLEND_FACTOR_ONE,
	BF_SRC_COLOR = VK_BLEND_FACTOR_SRC_COLOR,
	BF_ONE_MINUS_SRC_COLOR = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	BF_DST_COLOR = VK_BLEND_FACTOR_DST_COLOR,
	BF_ONE_MINUS_DST_COLOR = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	BF_SRC_ALPHA = VK_BLEND_FACTOR_SRC_ALPHA,
	BF_ONE_MINUS_SRC_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	BF_DST_ALPHA = VK_BLEND_FACTOR_DST_ALPHA,
	BF_ONE_MINUS_DST_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	BF_CONSTANT_COLOR = VK_BLEND_FACTOR_CONSTANT_COLOR,
	BF_ONE_MINUS_CONSTANT_COLOR = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	BF_CONSTANT_ALPHA = VK_BLEND_FACTOR_CONSTANT_ALPHA,
	BF_ONE_MINUS_CONSTANT_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	BF_SRC_ALPHA_SATURATE = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
	BF_SRC1_COLOR = VK_BLEND_FACTOR_SRC1_COLOR,
	BF_ONE_MINUS_SRC1_COLOR = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	BF_SRC1_ALPHA = VK_BLEND_FACTOR_SRC1_ALPHA,
	BF_ONE_MINUS_SRC1_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
};

struct StaticBlendState
{
	ColorWriteMask colorWriteMask = ColorWriteMask::CW_RGBA;

	BlendOperation	color_op = BlendOperation::BO_ADD;
	BlendFactor	color_src_factor = BlendFactor::BF_ONE;
	BlendFactor	color_dest_factor = BlendFactor::BF_ONE;

	BlendOperation	alpha_op = BlendOperation::BO_ADD;
	BlendFactor	alpha_src_factor = BlendFactor::BF_ONE;
	BlendFactor	alpha_dest_factor = BlendFactor::BF_ONE;
	uint32_t		outputAttachmentCount = 1;
	StaticBlendState(
		uint32_t		_outputAttachmentCount = 1,
		ColorWriteMask _colorWriteMask = ColorWriteMask::CW_RGBA,
		BlendOperation	_color_op = BlendOperation::BO_ADD,
		BlendFactor	_color_src_factor = BlendFactor::BF_ONE,
		BlendFactor	_color_dest_factor = BlendFactor::BF_ONE,
		BlendOperation _alpha_op = BlendOperation::BO_ADD,
		BlendFactor	_alpha_src_factor = BlendFactor::BF_ONE,
		BlendFactor	_alpha_dest_factor = BlendFactor::BF_ONE
	) :
		colorWriteMask(_colorWriteMask),
		color_op(_color_op),
		color_src_factor(_color_src_factor),
		color_dest_factor(_color_dest_factor),
		alpha_op(_alpha_op),
		alpha_src_factor(_alpha_src_factor),
		alpha_dest_factor(_alpha_dest_factor),
		outputAttachmentCount(_outputAttachmentCount)
	{}
};

enum PolygonMode {
	PM_FILL = VkPolygonMode::VK_POLYGON_MODE_FILL,
	PM_LINE = VkPolygonMode::VK_POLYGON_MODE_LINE,
	PM_POINT = VkPolygonMode::VK_POLYGON_MODE_POINT,
};

enum CullMode {
	ECM_NONE = VkCullModeFlagBits::VK_CULL_MODE_NONE,
	ECM_FRONT = VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT,
	ECM_BACK = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT,
	ECM_FRONT_AND_BACK = VkCullModeFlagBits::VK_CULL_MODE_FRONT_AND_BACK,
};

enum PrimitiveTopology {
	PT_POINT_LIST = 0,
	PT_LINE_LIST = 1,
	PT_LINE_STRIP = 2,
	PT_TRIANGLE_LIST = 3,
	PT_TRIANGLE_STRIP = 4,
	PT_TRIANGLE_FAN = 5,
	PT_LINE_LIST_WITH_ADJACENCY = 6,
	PT_LINE_STRIP_WITH_ADJACENCY = 7,
	PT_TRIANGLE_LIST_WITH_ADJACENCY = 8,
	PT_TRIANGLE_STRIP_WITH_ADJACENCY = 9,
	PT_PATCH_LIST = 10,
};

struct Rasterizer
{
	PolygonMode			polygonMode = PolygonMode::PM_FILL;
	CullMode			cullMode = CullMode::ECM_BACK;
	PrimitiveTopology	primitiveTopology = PrimitiveTopology::PT_TRIANGLE_LIST;
	Rasterizer(
		PolygonMode			pm = PolygonMode::PM_FILL,
		CullMode			cm = CullMode::ECM_BACK,
		PrimitiveTopology	pt = PrimitiveTopology::PT_TRIANGLE_LIST) :
		polygonMode(pm),
		cullMode(cm),
		primitiveTopology(pt)
	{}
};

enum CompareOp {
	CO_NEVER = VK_COMPARE_OP_NEVER,
	CO_LESS = VK_COMPARE_OP_LESS,
	CO_EQUAL = VK_COMPARE_OP_EQUAL,
	CO_LESS_OR_EQUAL = VK_COMPARE_OP_LESS_OR_EQUAL,
	CO_GREATER = VK_COMPARE_OP_GREATER,
	CO_NOT_EQUAL = VK_COMPARE_OP_NOT_EQUAL,
	CO_GREATER_OR_EQUAL = VK_COMPARE_OP_GREATER_OR_EQUAL,
	CO_ALWAYS = VK_COMPARE_OP_ALWAYS,
};

enum StencilOp {
	SO_KEEP = VK_STENCIL_OP_KEEP,
	SO_ZERO = VK_STENCIL_OP_ZERO,
	SO_REPLACE = VK_STENCIL_OP_REPLACE,
	SO_INCREMENT_AND_CLAMP = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
	SO_DECREMENT_AND_CLAMP = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
	SO_INVERT = VK_STENCIL_OP_INVERT,
	SO_INCREMENT_AND_WRAP = VK_STENCIL_OP_INCREMENT_AND_WRAP,
	SO_DECREMENT_AND_WRAP = VK_STENCIL_OP_DECREMENT_AND_WRAP,
};

struct DepthStencil
{
	bool bEnableDepth = true;
	bool bEnableDepthTest = true;
	CompareOp depthCompareOp = CompareOp::CO_LESS_OR_EQUAL;
	bool bEnableStencilTest = false;
	CompareOp stencilCompareOp = CompareOp::CO_ALWAYS;
	StencilOp stencilTestFailOp = StencilOp::SO_KEEP;
	StencilOp stencilTestPassOp = StencilOp::SO_KEEP;
	StencilOp stencilDepthFailOp = StencilOp::SO_KEEP;
	uint32_t reference = 1;

	DepthStencil(
		bool _bEnableDepth = true,
		bool _bEnableDepthTest = true,
		CompareOp _depthCompareOp = CompareOp::CO_LESS_OR_EQUAL,
		bool _bEnableStencilTest = false,
		CompareOp _stencilCompareOp = CompareOp::CO_ALWAYS,
		StencilOp _stencilTestFailOp = StencilOp::SO_KEEP,
		StencilOp _stencilTestPassOp = StencilOp::SO_KEEP,
		StencilOp _stencilDepthFailOp = StencilOp::SO_KEEP,
		uint32_t _reference = 1
	) :
		bEnableDepth(_bEnableDepth),
		bEnableDepthTest(_bEnableDepthTest),
		depthCompareOp(_depthCompareOp),
		bEnableStencilTest(_bEnableStencilTest),
		stencilCompareOp(_stencilCompareOp),
		stencilTestFailOp(_stencilTestFailOp),
		stencilTestPassOp(_stencilTestPassOp),
		stencilDepthFailOp(_stencilDepthFailOp),
		reference(_reference)
	{}
};

enum class PipelineType
{
	Graphics,
	Compute
};

struct PipelineObject
{
	VkPipeline	pipeline;

	VkPipelineLayout layout;

	PipelineType pipelineType;

	bool bHasMaterialParameter = false;
	bool bHasMaterialTexture = false;

	~PipelineObject();
};

struct PipelineIndex
{
	uint64_t vsIndex;
	uint64_t psIndex;
	uint64_t varients;
	uint32_t layout;
	bool operator<(const PipelineIndex& id) const {
		return (vsIndex < id.vsIndex) 
			&& (psIndex < id.psIndex) 
			&& (varients < id.varients) 
			&& (layout < id.layout);
	}

	bool operator==(const PipelineIndex& id) const {
		return (vsIndex == id.vsIndex)
			&& (psIndex == id.psIndex)
			&& (varients == id.varients)
			&& (layout == id.layout);
	}
};

class PipelineManager
{
public:

	PipelineManager();
	~PipelineManager();

	//Graphics pipeline setting step 1
	static void SetColorBlend(VkGraphicsPipelineCreateInfoCache & createInfo, bool bEnable, StaticBlendState blendState = StaticBlendState());

	//Graphics pipeline setting step 2
	static void SetRenderRasterizer(VkGraphicsPipelineCreateInfoCache& createInfo, Rasterizer rasterizer = Rasterizer());

	//Graphics pipeline setting step 3
	static void SetRenderDepthStencil(VkGraphicsPipelineCreateInfoCache& createInfo, DepthStencil depthStencil = DepthStencil());

	//Graphics pipeline setting step 4
	static void SetVertexInput(VkGraphicsPipelineCreateInfoCache& createInfo, VertexInputLayout vertexInputLayout);
	static void SetVertexInput(VkGraphicsPipelineCreateInfoCache& createInfo, uint32_t vertexInputStride ,VkVertexInputRate vertexInputRate,std::vector<VkFormat>inputLayout );

	//Graphics pipeline setting step 5 , also not.
	static void SetDepthStencil(VkGraphicsPipelineCreateInfoCache& createInfo);

	//Graphics pipeline setting step 6
	static void SetVertexShaderAndPixelShader(VkGraphicsPipelineCreateInfoCache& createInfo, ShaderCache vs, ShaderCache ps);

	//Graphics pipeline setting the last step
	static PipelineObject* CreatePipelineObject(
		VkGraphicsPipelineCreateInfoCache& createInfo,VkPipelineLayout layout, 
		VkRenderPass renderPass, PipelineIndex pipelineIndex, 
		uint32_t subpassCount = 1, PipelineType pipelineType = PipelineType::Graphics);

	static PipelineObject* GetGraphicsPipelineMap(PipelineIndex index);

	static void ClearPipelineObjects();

	static void BuildGraphicsPipelineState(VkGraphicsPipelineCreateInfoCache& createInfo, VkRenderPass renderPass, uint32_t subpassIndex, VkPipeline& pipelineObj);

	static void SetPipelineLayout(VkGraphicsPipelineCreateInfoCache& createInfo, VkPipelineLayout pipelineLayout);

	static void ClearCreateInfo(VkGraphicsPipelineCreateInfoCache& createInfo);
private:
	static std::map<PipelineIndex, std::unique_ptr<PipelineObject>> _graphicsPipelines;
	static std::map<PipelineIndex, std::unique_ptr<PipelineObject>> _computePipelines;

};