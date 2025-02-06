#pragma once
#include "Common.h"
#include "VulkanManager.h"
#include <map>
#include "./Asset/HGuid.h"
enum class MPType : uint8_t
{
	VSFloat = 0,
	VSFloat2 = 1,
	VSFloat3 = 2,
	VSFloat4 = 3,
	PSFloat = 4,
	PSFloat2 = 5,
	PSFloat3 = 6,
	PSFloat4 = 7
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
	HideInEditor = 0x00000002,		//Shader will not be show in Editor.
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
	//{ pos,nor,tan,col,uv01,uv23,uv45} //第一个pos必须有
	uint8_t vertexInput[7] = {3,0,0,0,0,0,0};
	//Shader parameter count
	uint8_t shaderParameterCount_vs = 0;
	uint8_t shaderParameterCount_ps = 0;
	//Shader texture count
	uint8_t shaderTextureCount = 0;
	//Shader varients
	uint8_t varientCount = 0;
	//Shader SV_Target Count
	uint8_t colorAttachmentCount = 0;
};

struct ShaderParameterInfo
{
	MPType type;
	char name[32]="\0";
	char group[16] = "Default\0";
	glm::vec4 defaultValue;
	uint16_t index = 0;
};

struct ShaderTextureInfo
{
	MTType type;
	char name[32] = "\0";
	char group[16] = "Default\0";
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
	//同名顶点着色器
	std::weak_ptr<ShaderCache> vsShader;
	VkShaderModule shaderModule;
	std::string shaderName;
	std::string shaderFullName;
	//RelativePath
	std::string shaderPath;
	//AbsPath
	std::string shaderAbsPath;
	VkPipelineShaderStageCreateInfo shaderStageInfo={};
	//Default 默认参数模板
	std::vector<ShaderVarientGroup> vi;
	uint32_t varients;
	std::vector<std::shared_ptr<struct MaterialParameterInfo>> pi_vs;
	std::vector<std::shared_ptr<struct MaterialParameterInfo>> pi_ps;
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
	HBBR_API static void ReloadMaterialShaderCacheAndPipelineObject(std::weak_ptr<class Material>mat);

	HBBR_API static void DestroyAllShaderCache();

	HBBR_API inline static std::weak_ptr<ShaderCache> GetVSCache(std::string cacheFullName) {
		auto it = _vsShader.find(cacheFullName);
		if(it != _vsShader.end())
		{ 
			return it->second;
		}
		return std::weak_ptr<ShaderCache>();
	}

	HBBR_API inline static std::weak_ptr<ShaderCache> GetPSCache(std::string cacheFullName) {
		auto it = _psShader.find(cacheFullName);
		if (it != _psShader.end())
		{
			return it->second;
		}
		return std::weak_ptr<ShaderCache>();
	}

	HBBR_API inline static  std::map<std::string, std::shared_ptr<ShaderCache>> GetPSCaches() {
		return _psShader;
	}

	HBBR_API inline static std::weak_ptr<ShaderCache> GetCSCache(std::string cacheFullName) {
		auto it = _csShader.find(cacheFullName);
		if (it != _csShader.end())
		{
			return it->second;
		}
		return std::weak_ptr<ShaderCache>();
	}
	static std::map<std::string, std::shared_ptr<ShaderCache>> _vsShader;
	static std::map<std::string, std::shared_ptr<ShaderCache>> _psShader;
	static std::map<std::string, std::shared_ptr<ShaderCache>> _csShader;
};



