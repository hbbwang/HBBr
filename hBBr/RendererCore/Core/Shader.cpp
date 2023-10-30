#include "Shader.h"
#include "FileSystem.h"
#include "VulkanManager.h"
#include "./Component/Material.h"
#include <fstream>
std::map<HString, ShaderCache> Shader::_vsShader;
std::map<HString, ShaderCache> Shader::_psShader;
std::map<HString, ShaderCache> Shader::_csShader;

void Shader::LoadShaderCache(const char* cachePath)
{
	auto allCacheFiles = FileSystem::GetFilesBySuffix(cachePath, "spv");
	uint64_t cacheIndex = 0;
	for (auto i : allCacheFiles)
	{
		HString fileName = i.baseName;
		auto split = fileName.Split("@");
		ShaderCache cache = {};
		cache.shaderCacheIndex = cacheIndex;
		//
		//auto shaderData = FileSystem::ReadBinaryFile(i.absPath.c_str());
		std::ifstream file(i.absPath.c_str(), std::ios::ate | std::ios::binary);
		size_t fileSize = static_cast<size_t>(file.tellg());
		file.seekg(0);
		//header
		file.read((char*)&cache.header, sizeof(ShaderCacheHeader));
		//shader parameter infos
		for (int i = 0; i < cache.header.shaderParameterCount; i++)
		{
			ShaderParameterInfo newInfo;
			file.read((char*)&newInfo, sizeof(ShaderParameterInfo));
			cache.params.push_back(newInfo);
		}
		//shader textures infos
		for (int i = 0; i < cache.header.shaderTextureCount; i++)
		{
			ShaderTextureInfo newInfo;
			file.read((char*)&newInfo, sizeof(ShaderTextureInfo));
			cache.texs.push_back(newInfo);
		}
		//cache
		size_t shaderCodeSize =
			fileSize 
			- sizeof(ShaderCacheHeader) 
			- (sizeof(ShaderParameterInfo) * cache.header.shaderParameterCount)
			- (sizeof(ShaderTextureInfo) * cache.header.shaderTextureCount)
			;
		std::vector<char> shaderData(shaderCodeSize);
		file.read(shaderData.data(), shaderCodeSize);
		VulkanManager::GetManager()->CreateShaderModule(shaderData, cache.shaderModule);
		file.close();
		//
		cache.shaderPath = i.relativePath;
		cache.shaderName = split[0];
		cache.shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		cache.shaderStageInfo.module = cache.shaderModule;
		//创建模板
		//ParameterInfo
		{
			int alignmentFloat4 = 0; // float4 对齐
			uint32_t arrayIndex = 0;
			for (auto i = 0; i < cache.params.size(); i++)
			{
				std::shared_ptr<MaterialParameterInfo> info;
				info.reset(new MaterialParameterInfo);
				info->name = cache.params[i].name;
				info->type = (MPType)cache.params[i].type;
				if (info->type == MPType::Float)
				{
					if (alignmentFloat4 + 1 > 4)
					{
						alignmentFloat4 = 0;
						arrayIndex++;
					}
					info->arrayIndex = arrayIndex;
					info->vec4Index = alignmentFloat4;
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4]);
					alignmentFloat4 += 1;
				}
				else if (info->type == MPType::Float2)
				{
					if (alignmentFloat4 + 2 > 4)
					{
						alignmentFloat4 = 0;
						arrayIndex++;
					}
					info->arrayIndex = arrayIndex;
					info->vec4Index = alignmentFloat4;
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4]);
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4 + 1]);
					alignmentFloat4 += 2;
				}
				else if (info->type == MPType::Float3)
				{
					if (alignmentFloat4 + 3 > 4)
					{
						alignmentFloat4 = 0;
						arrayIndex++;
					}
					info->arrayIndex = arrayIndex;
					info->vec4Index = alignmentFloat4;
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4]);
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4 + 1]);
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4 + 2]);
					alignmentFloat4 += 3;
				}
				else if (info->type == MPType::Float4)
				{
					if (alignmentFloat4 + 4 > 4)
					{
						alignmentFloat4 = 0;
						arrayIndex++;
					}
					info->arrayIndex = arrayIndex;
					info->vec4Index = alignmentFloat4;
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4]);
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4 + 1]);
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4 + 2]);
					info->value.push_back((float)cache.params[i].defaultValue[alignmentFloat4 + 3]);
					alignmentFloat4 += 4;
				}
				cache.pi.push_back(info);
			}
		}
		//TextureInfo
		{
			for (auto i = 0; i < cache.texs.size(); i++)
			{
				std::shared_ptr<MaterialTextureInfo> info;
				info.reset(new MaterialTextureInfo);
				info->name = cache.texs[i].name;
				info->type = (MTType)cache.texs[i].type;
				info->index = cache.texs[i].index;
				info->value = cache.texs[i].defaultTexture;
				info->samplerAddress = cache.texs[i].msAddress;
				info->samplerFilter = cache.texs[i].msFilter;
				cache.ti.push_back(info);
			}
		}
		//
		if (split[1] == "vs")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			cache.shaderStageInfo.pName = "VSMain";
			cache.shaderType = ShaderType::VertexShader;
			_vsShader.emplace(std::make_pair(cache.shaderName, cache));
		}
		else if (split[1] == "ps")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			cache.shaderStageInfo.pName = "PSMain";
			cache.shaderType = ShaderType::PixelShader;
			_psShader.emplace(std::make_pair(cache.shaderName, cache));
		}
		else if (split[1] == "cs")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			cache.shaderStageInfo.pName = "CSMain";
			cache.shaderType = ShaderType::ComputeShader;
			_csShader.emplace(std::make_pair(cache.shaderName, cache));
		}

		cacheIndex++;
	}

}

void Shader::DestroyAllShaderCache()
{
	for (auto& i : _vsShader)
	{
		vkDestroyShaderModule(VulkanManager::GetManager()->GetDevice(), i.second.shaderModule, VK_NULL_HANDLE);
	}
	for (auto& i : _psShader)
	{
		vkDestroyShaderModule(VulkanManager::GetManager()->GetDevice(), i.second.shaderModule, VK_NULL_HANDLE);
	}
	for (auto& i : _csShader)
	{
		vkDestroyShaderModule(VulkanManager::GetManager()->GetDevice(), i.second.shaderModule, VK_NULL_HANDLE);
	}
	_vsShader.clear();
	_psShader.clear();
	_csShader.clear();
}
