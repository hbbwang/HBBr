#pragma once
#include "Common.h"
#include "HString.h"
#include <vulkan/vulkan.h>
#include <map>

enum class MPType : uint8_t
{
	Unknow = 0,
	Float = 1,
	Float2 = 2,
	Float3 = 3,
	Float4 = 4
};

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
	//Vertex input layout type,vs only
	//{ pos,nor,tan,col,uv01,uv23 }
	uint8_t vertexInput[6] = {0,0,0,0,0,0};
	//Shader parameter count
	uint8_t shaderParameterCount = 0;
};

struct ShaderParameterInfo
{
	MPType type;
	char name[32];
	glm::vec4 defaultValue;
};

struct ShaderCache
{
	ShaderCacheHeader header;
	std::vector<ShaderParameterInfo> params;
	//
	ShaderType shaderType;
	VkShaderModule shaderModule;
	HString shaderName;
	HString shaderPath;
	VkPipelineShaderStageCreateInfo shaderStageInfo={};
	uint64_t shaderCacheIndex = 0;
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



