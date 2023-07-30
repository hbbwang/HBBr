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

typedef uint32_t ShaderFlags;
typedef enum ShaderFlagBits
{
	EnableShaderDebug = 0x00000001,		//Useful for shader debug in renderdoc .
	HiddenInMaterial = 0x00000002,		//Shader will not be show in Material.
}ShaderFlagBits;

struct ShaderCacheHeader
{
	ShaderFlags flags;
};

struct ShaderCache
{
	ShaderCacheHeader header;
	//
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



