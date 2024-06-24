#pragma once
#include "Common.h"
#include "HString.h"
#include "VulkanManager.h"
#include <map>
#include "./Asset/HGuid.h"
enum class MPType : uint8_t
{
	Float = 0,
	Float2 = 1,
	Float3 = 2,
	Float4 = 3
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

struct ShaderVarientGroup {
	char name[31] = "\0";
	char defaultValue = '\0';//bool
};

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
	//Shader varients
	uint8_t varientCount = 0;
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
	//<varients,VkShaderModule>
	VkShaderModule shaderModule;
	HString shaderName;
	HString shaderFullName;
	//RelativePath
	HString shaderPath;
	//AbsPath
	HString shaderAbsPath;
	VkPipelineShaderStageCreateInfo shaderStageInfo={};
	//Default 默认参数模板
	std::vector<ShaderVarientGroup> vi;
	uint32_t varients;
	std::vector<std::shared_ptr<struct MaterialParameterInfo>>  pi;
	std::vector<std::shared_ptr<struct MaterialTextureInfo>> ti;
};

class Shader
{
public:
	//加载所有Cache,如果已经加载了，会销毁掉重新加载
	HBBR_API static void LoadShaderCache(const char* cachePath);
	//根据Shader名字加载Cache (@前)，会销毁掉重新加载
	HBBR_API static void LoadShaderCacheByShaderName(const char* cachePath, const char* ShaderName,ShaderType type);
	//加载单个Cache，会销毁掉重新加载
	HBBR_API static void LoadShaderCacheFile(const char* cacheFilePath);
	//Material ReloadCache & pipeline
	HBBR_API static void ReloadMaterialShaderCacheAndPipelineObject(std::weak_ptr<Material>mat);

	HBBR_API static void DestroyAllShaderCache();

	HBBR_API inline static std::weak_ptr<ShaderCache> GetVSCache(HString cacheFullName) {
		auto it = _vsShader.find(cacheFullName);
		if(it != _vsShader.end())
		{ 
			return it->second;
		}
		return std::weak_ptr<ShaderCache>();
	}

	HBBR_API inline static std::weak_ptr<ShaderCache> GetPSCache(HString cacheFullName) {
		auto it = _psShader.find(cacheFullName);
		if (it != _psShader.end())
		{
			return it->second;
		}
		return std::weak_ptr<ShaderCache>();
	}

	HBBR_API inline static std::weak_ptr<ShaderCache> GetCSCache(HString cacheFullName) {
		auto it = _csShader.find(cacheFullName);
		if (it != _csShader.end())
		{
			return it->second;
		}
		return std::weak_ptr<ShaderCache>();
	}

	static std::map<HString, std::shared_ptr<ShaderCache>> _vsShader;
	static std::map<HString, std::shared_ptr<ShaderCache>> _psShader;
	static std::map<HString, std::shared_ptr<ShaderCache>> _csShader;
};



