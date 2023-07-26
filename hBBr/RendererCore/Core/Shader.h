#pragma once
#include "Common.h"
#include "HString.h"
#include <vulkan/vulkan.h>
#include <map>

enum class ShaderType
{
	VertexShader = 0,
	PixelShader = 1,
	ComputeShader = 2,
};

struct ShaderCache
{
	ShaderType shaderType;
	VkShaderModule shaderModule;
	HString shaderName;
	HString shaderPath;
	VkPipelineShaderStageCreateInfo shaderStageInfo={};
};

class Shader
{
public:
	HBBR_API static void LoadShaderCache(const char* cachePath);
	HBBR_API static void DestroyAllShaderCache();
	static std::map<HString, ShaderCache> _vsShader;
	static std::map<HString, ShaderCache> _psShader;
	static std::map<HString, ShaderCache> _csShader;
};



