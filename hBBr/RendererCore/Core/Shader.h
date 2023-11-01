#pragma once
#include "Common.h"
#include "HString.h"
#include "VulkanManager.h"
#include <map>
#include "./Resource/HGuid.h"
enum class MPType : uint8_t
{
	Unknow = 0,
	Float = 1,
	Float2 = 2,
	Float3 = 3,
	Float4 = 4
};

enum class MTType : uint8_t
{
	Unknow = 0,
	Texture2D = 1,
	Texture3D = 2,
	TextureCube = 3,
	TextureArray = 4
};

enum class MSFilter : uint8_t
{
	Unknow = 0,
	Linear = 1,
	Nearest = 2,
};

enum class MSAddress : uint8_t
{
	Unknow = 0,
	Clamp = 1,
	Wrap = 2,
	Mirror = 3,
	Border =4,
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
	NativeHLSL = 0x00000004,			//Native HLSL, no extended compilation, need to follow the HLSL code specification

}ShaderFlagBits;

struct ShaderCacheHeader
{
	ShaderFlags flags;
	//Vertex input layout type,vs only
	//{ pos,nor,tan,col,uv01,uv23 }
	uint8_t vertexInput[6] = {0,0,0,0,0,0};
	//Shader parameter count
	uint8_t shaderParameterCount = 0;
	//Shader texture count
	uint8_t shaderTextureCount = 0;
	//Shader varients(32bit)
	uint32_t varients = 0;
};

struct ShaderParameterInfo
{
	MPType type;
	char name[32]="\0";
	glm::vec4 defaultValue;
	uint16_t index = 0;
};

struct ShaderTextureInfo
{
	MPType type;
	char name[32] = "\0";
	char defaultTexture[16];//systemTexture
	uint8_t index = 0;
	MSFilter msFilter = MSFilter::Linear;
	MSAddress msAddress = MSAddress::Wrap;
};

struct ShaderCache
{
	ShaderCacheHeader header;
	std::vector<ShaderParameterInfo> params;
	std::vector<ShaderTextureInfo> texs;
	//
	ShaderType shaderType;
	VkShaderModule shaderModule;
	HString shaderName;
	HString shaderPath;
	VkPipelineShaderStageCreateInfo shaderStageInfo={};
	uint32_t shaderLoadIndex = 0;
	uint32_t varients = 0;
	//Default 默认参数模板
	std::vector<std::shared_ptr<struct MaterialParameterInfo>>  pi;
	std::vector<std::shared_ptr<struct MaterialTextureInfo>> ti;
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



