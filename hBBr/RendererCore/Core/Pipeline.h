#pragma once
//#include "Common.h"
#include <vector>
#include "VulkanManager.h"
#include "Shader.h"
#include "Pass/PassType.h"
#include "ConsoleDebug.h"
#include "RendererConfig.h"

struct VkGraphicsPipelineCreateInfoCache
{
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

	bool bHasMaterialParameterVS = false;
	bool bHasMaterialParameterPS = false;
	bool bHasMaterialTexture = false;
	
};

struct VkComputePipelineCreateInfoCache
{
	VkComputePipelineCreateInfo			createInfo{};
	bool bHasParameter = false;
	bool bHasStoreTexture = false;
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

enum class BlendMode
{
	Opaque = 0,
	Transparent = 1,
	Additive = 2,
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

	bool bHasMaterialParameterVS = false;
	bool bHasMaterialParameterPS = false;
	bool bHasMaterialTexture = false;
	bool bHasStoreTexture = false;
	~PipelineObject();
};

class PipelineIndex
{
	friend class Material;
public:

	HBBR_INLINE HString GetVSShaderFullName()
	{
		return vsShaderCacheFullName;
	}

	HBBR_INLINE HString GetPSShaderFullName()
	{
		return psShaderCacheFullName;
	}

	HBBR_INLINE HString GetVSShaderName()
	{
		return vsShaderName;
	}

	HBBR_INLINE HString GetPSShaderName()
	{
		return psShaderName;
	}

	HBBR_INLINE uint32_t GetVSVarient()
	{
		return ps_varients;
	}

	HBBR_INLINE uint32_t GetPSVarient()
	{
		return vs_varients;
	}

	HBBR_INLINE void SetVSVarient(uint32_t new_vs_varient)
	{
		vs_varients = new_vs_varient;
		vsShaderCacheFullName = vsShaderName + "@" + HString::FromUInt(new_vs_varient);
	}

	HBBR_INLINE void SetPSVarient(uint32_t new_ps_varient)
	{
		ps_varients = new_ps_varient;
		psShaderCacheFullName = psShaderName + "@" + HString::FromUInt(new_ps_varient);
	}

	HBBR_INLINE void SetVarient(uint32_t new_vs_varient, uint32_t new_ps_varient)
	{
		SetVSVarient(new_vs_varient);
		SetPSVarient(new_ps_varient);
	}

	HBBR_INLINE BlendMode GetBlendMode()const
	{
		return blendMode;
	}

	HBBR_INLINE PipelineType GetPipelineType()const
	{
		return type;
	}

	static PipelineIndex GetPipelineIndex(
		std::weak_ptr<ShaderCache> vs,
		std::weak_ptr<ShaderCache> ps)
	{
		PipelineIndex index;
		index.vs_varients = vs.lock()->varients;
		index.ps_varients = ps.lock()->varients;
		index.blendMode = BlendMode::Opaque;
		index.type = PipelineType::Graphics;
		index.vsShaderCacheFullName = vs.lock()->shaderFullName;
		index.psShaderCacheFullName = ps.lock()->shaderFullName;
		index.vsShaderName = vs.lock()->shaderName;
		index.psShaderName = ps.lock()->shaderName;
		return index;
	}

	static PipelineIndex GetPipelineIndex(
		std::weak_ptr<ShaderCache> cs)
	{
		PipelineIndex index;
		index.vs_varients = cs.lock()->varients;
		index.ps_varients = cs.lock()->varients;
		index.type = PipelineType::Compute;
		index.vsShaderCacheFullName = cs.lock()->shaderFullName;
		index.psShaderCacheFullName = cs.lock()->shaderFullName;
		index.vsShaderName = cs.lock()->shaderName;
		index.psShaderName = cs.lock()->shaderName;
		return index;
	}

	bool operator<(const PipelineIndex& id) const {
		return (vs_varients < id.vs_varients)
			|| (ps_varients < id.ps_varients)
			|| (blendMode < id.blendMode)
			|| (type < id.type)
			|| vsShaderCacheFullName< id.vsShaderCacheFullName
			|| psShaderCacheFullName < id.psShaderCacheFullName;
	}

	bool operator==(const PipelineIndex& id) const {
		return  (vs_varients == id.vs_varients)
			&& (ps_varients == id.ps_varients)
			&& (blendMode == id.blendMode)
			&& (type == id.type)
			&& (vsShaderCacheFullName == id.vsShaderCacheFullName)
			&& (psShaderCacheFullName == id.psShaderCacheFullName);
	}

private:
	BlendMode blendMode = BlendMode::Opaque;
	PipelineType type = PipelineType::Graphics;
	uint32_t vs_varients = 0;//变体 32bit 相当于32个bool
	uint32_t ps_varients = 0;
	HString vsShaderCacheFullName = "";
	HString psShaderCacheFullName = "";
	HString vsShaderName = "";
	HString psShaderName = "";
};

class PipelineManager
{
	friend class VulkanApp;
public:

	PipelineManager();
	~PipelineManager();

	//Graphics pipeline setting step 1
	HBBR_API static void SetColorBlend(VkGraphicsPipelineCreateInfoCache & createInfo, bool bEnable, StaticBlendState blendState = StaticBlendState());

	//Graphics pipeline setting step 2
	HBBR_API static void SetRenderRasterizer(VkGraphicsPipelineCreateInfoCache& createInfo, Rasterizer rasterizer = Rasterizer());

	//Graphics pipeline setting step 3
	HBBR_API static void SetRenderDepthStencil(VkGraphicsPipelineCreateInfoCache& createInfo, DepthStencil depthStencil = DepthStencil());

	//Graphics pipeline setting step 4
	HBBR_API static void SetVertexInput(VkGraphicsPipelineCreateInfoCache& createInfo, VertexInputLayout vertexInputLayout);
	HBBR_API static void SetVertexInput(VkGraphicsPipelineCreateInfoCache& createInfo, uint32_t vertexInputStride ,VkVertexInputRate vertexInputRate,std::vector<VkFormat>inputLayout );

	//Graphics pipeline setting step 5 , also not.
	HBBR_API static void SetDepthStencil(VkGraphicsPipelineCreateInfoCache& createInfo);

	//Graphics pipeline setting step 6
	HBBR_API static void SetVertexShaderAndPixelShader(VkGraphicsPipelineCreateInfoCache& createInfo, ShaderCache* vs, ShaderCache* ps);

	HBBR_API static void SetComputeShader(VkComputePipelineCreateInfoCache& createInfo, ShaderCache* cs);

	//Graphics pipeline setting the last step
	HBBR_API static PipelineObject* CreatePipelineObject(
		VkGraphicsPipelineCreateInfoCache& createInfo,VkPipelineLayout layout, 
		VkRenderPass renderPass, PipelineIndex pipelineIndex, 
		uint32_t subpassCount = 1, PipelineType pipelineType = PipelineType::Graphics);

	HBBR_API static PipelineObject* GetGraphicsPipelineMap(PipelineIndex index);

	HBBR_API static PipelineObject* GetComputePipelineMap(PipelineIndex index);

	HBBR_API static void ClearPipelineObjects();

	HBBR_API static void RemovePipelineObjects(PipelineIndex& index);

	HBBR_API static void BuildGraphicsPipelineState(VkGraphicsPipelineCreateInfoCache& createInfo, VkRenderPass renderPass, uint32_t subpassIndex, VkPipeline& pipelineObj);

	HBBR_API static void BuildComputePipelineState(VkComputePipelineCreateInfoCache& createInfo,  VkPipeline& pipelineObj);

	HBBR_API static void SetPipelineLayout(VkGraphicsPipelineCreateInfoCache& createInfo, VkPipelineLayout pipelineLayout);

	HBBR_API static void SetPipelineLayout(VkComputePipelineCreateInfoCache& createInfo, VkPipelineLayout pipelineLayout);

	HBBR_API static void ClearCreateInfo(VkGraphicsPipelineCreateInfoCache& createInfo);

	HBBR_API static PipelineIndex AddPipelineObject(std::weak_ptr<ShaderCache> vs, std::weak_ptr<ShaderCache> ps,VkPipeline pipeline,VkPipelineLayout pipelineLayout);

	HBBR_API static PipelineIndex AddPipelineObject(std::weak_ptr<ShaderCache> cs, VkPipeline pipeline, VkPipelineLayout pipelineLayout);
private:
	static std::map<PipelineIndex, std::unique_ptr<PipelineObject>> _graphicsPipelines;
	static std::map<PipelineIndex, std::unique_ptr<PipelineObject>> _computePipelines;

	static void GlobalInit();
	static void GlobalRelease();

	// 公共Layout布局模板，可直接调用
	
	//
	static VkDescriptorSetLayout _descriptorSetLayout_vs_ubd;
	static VkDescriptorSetLayout _descriptorSetLayout_ps_ubd;
	static VkDescriptorSetLayout _descriptorSetLayout_vsps_ubd;

	static uint8_t _maxTextureBinding;
	//Texture DescriptorSetLayout
	static std::vector<VkDescriptorSetLayout> _descriptorSetLayout_tex;

	//Contain uniform buffers {pass,obj,vsmat,tex} and textures
	static std::vector<VkPipelineLayout> _pipelineLayout_p_o_vsm_t;
	//Contain uniform buffers {pass,obj,psmat,tex} and textures
	static std::vector<VkPipelineLayout> _pipelineLayout_p_o_psm_t;
	//Contain uniform buffers {pass,obj,vspsmat,tex} and textures
	static std::vector<VkPipelineLayout> _pipelineLayout_p_o_vspsm_t;
	//Contain uniform buffers {pass,obj,tex} and textures
	static std::vector<VkPipelineLayout> _pipelineLayout_p_o_t;

	//Contain uniform buffers {pass,obj,vsmat}
	static VkPipelineLayout _pipelineLayout_p_o_vsm;
	//Contain uniform buffers {pass,obj,psmat}
	static VkPipelineLayout _pipelineLayout_p_o_psm;
	//Contain uniform buffers {pass,obj,vspsmat}
	static VkPipelineLayout _pipelineLayout_p_o_vspsm;
	//Contain uniform buffers {pass,obj}
	static VkPipelineLayout _pipelineLayout_p_o;

public:

	// VkDescriptorSetLayout: 
	// Type = UNIFORM_BUFFER_DYNAMIC 
	// Stage = VertexShader
	static const HBBR_INLINE VkDescriptorSetLayout GetDescriptorSetLayout_UniformBufferDynamicVS() {
		return _descriptorSetLayout_vs_ubd;
	}
	// VkDescriptorSetLayout: 
	// Type = UNIFORM_BUFFER_DYNAMIC 
	// Stage = PixelShader
	static const HBBR_INLINE VkDescriptorSetLayout GetDescriptorSetLayout_UniformBufferDynamicPS() {
		return _descriptorSetLayout_ps_ubd;
	}
	// VkDescriptorSetLayout: 
	// Type = UNIFORM_BUFFER_DYNAMIC 
	// Stage = VertexShader and PixelShader
	static const HBBR_INLINE VkDescriptorSetLayout GetDescriptorSetLayout_UniformBufferDynamicVSPS() {
		return _descriptorSetLayout_vsps_ubd;
	}
	// VkDescriptorSetLayout: 
	// Type = COMBINED_IMAGE_SAMPLER 
	// Stage = VertexShader and PixelShader
	// bindingCount = How many textures want to bind.
	static const HBBR_INLINE VkDescriptorSetLayout GetDescriptorSetLayout_TextureSamplerVSPS(uint8_t bindingCount) {
		return _descriptorSetLayout_tex[bindingCount];
	}

	// VkPipelineLayout: 
	// DescriptorSetLayoutTypes =  { 
	//		UniformBufferDynamicVSPS (Pass UniformBuffer) , 
	//		UniformBufferDynamicVSPS (Object UniformBuffer) , 
	//		UniformBufferDynamicVS (MaterialVS UniformBuffer)
	// } 
	static const HBBR_INLINE VkPipelineLayout GetPipelineLayout_P_O_VSM() {
		return _pipelineLayout_p_o_vsm;
	}
	// VkPipelineLayout: 
	// DescriptorSetLayoutTypes =  { 
	//		UniformBufferDynamicVSPS (Pass UniformBuffer) , 
	//		UniformBufferDynamicVSPS (Object UniformBuffer) , 
	//		UniformBufferDynamicVS (MaterialPS UniformBuffer)
	// } 
	static const HBBR_INLINE VkPipelineLayout GetPipelineLayout_P_O_PSM() {
		return _pipelineLayout_p_o_psm;
	}
	// VkPipelineLayout: 
	// DescriptorSetLayoutTypes =  { 
	//		UniformBufferDynamicVSPS (Pass UniformBuffer) , 
	//		UniformBufferDynamicVSPS (Object UniformBuffer) , 
	//		UniformBufferDynamicVSPS (MaterialVSPS UniformBuffer)
	// } 
	static const HBBR_INLINE VkPipelineLayout GetPipelineLayout_P_O_VSPSM() {
		return _pipelineLayout_p_o_vspsm;
	}
	// VkPipelineLayout: 
	// DescriptorSetLayoutTypes =  { 
	//		UniformBufferDynamicVSPS (Pass UniformBuffer) , 
	//		UniformBufferDynamicVSPS (Object UniformBuffer) , 
	// } 
	static const HBBR_INLINE VkPipelineLayout GetPipelineLayout_P_O() {
		return _pipelineLayout_p_o;
	}
	// VkPipelineLayout: 
	// DescriptorSetLayoutTypes =  { 
	//		UniformBufferDynamicVSPS (Pass UniformBuffer) , 
	//		UniformBufferDynamicVSPS (Object UniformBuffer) , 
	//		UniformBufferDynamicVS (MaterialVS UniformBuffer)
	//		TextureSamplerVSPS (Material Textures and Samplers)
	// } 
	static const HBBR_INLINE VkPipelineLayout GetPipelineLayout_P_O_VSM_T(uint8_t bindingCount) {
		return _pipelineLayout_p_o_vsm_t[bindingCount];
	}
	// VkPipelineLayout: 
	// DescriptorSetLayoutTypes =  { 
	//		UniformBufferDynamicVSPS (Pass UniformBuffer) , 
	//		UniformBufferDynamicVSPS (Object UniformBuffer) , 
	//		UniformBufferDynamicPS (MaterialPS UniformBuffer)
	//		TextureSamplerVSPS (Material Textures and Samplers)
	// } 
	static const HBBR_INLINE VkPipelineLayout GetPipelineLayout_P_O_PSM_T(uint8_t bindingCount) {
		return _pipelineLayout_p_o_psm_t[bindingCount];
	}
	// VkPipelineLayout: 
	// DescriptorSetLayoutTypes =  { 
	//		UniformBufferDynamicVSPS (Pass UniformBuffer) , 
	//		UniformBufferDynamicVSPS (Object UniformBuffer) , 
	//		UniformBufferDynamicVSPS (MaterialVSPS UniformBuffer)
	//		TextureSamplerVSPS (Material Textures and Samplers)
	// } 
	static const HBBR_INLINE VkPipelineLayout GetPipelineLayout_P_O_VSPSM_T(uint8_t bindingCount) {
		return _pipelineLayout_p_o_vspsm_t[bindingCount];
	}
	// VkPipelineLayout: 
	// DescriptorSetLayoutTypes =  { 
	//		UniformBufferDynamicVSPS (Pass UniformBuffer) , 
	//		UniformBufferDynamicVSPS (Object UniformBuffer) , 
	//		TextureSamplerVSPS (Material Textures and Samplers)
	// } 
	static const HBBR_INLINE VkPipelineLayout GetPipelineLayout_P_O_T(uint8_t bindingCount) {
		return _pipelineLayout_p_o_t[bindingCount];
	}
};