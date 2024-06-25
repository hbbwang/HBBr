#include "Shader.h"
#include "FileSystem.h"
#include "VulkanManager.h"
#include "./Asset/Material.h"
#if IS_EDITOR
#include "ShaderCompiler.h"
#endif
#include <fstream>
std::map<HString, std::shared_ptr<ShaderCache>> Shader::_vsShader;
std::map<HString, std::shared_ptr<ShaderCache>> Shader::_psShader;
std::map<HString, std::shared_ptr<ShaderCache>> Shader::_csShader;

void Shader::LoadShaderCache(const char* cachePath)
{
	auto allCacheFiles = FileSystem::GetFilesBySuffix(cachePath, "spv");
	for (auto i : allCacheFiles)
	{
		Shader::LoadShaderCacheFile(i.absPath.c_str());
	}
}

void Shader::LoadShaderCacheByShaderName(const char* cachePath, const char* ShaderName, ShaderType type)
{
	auto allCacheFiles = FileSystem::GetFilesBySuffix(cachePath, "spv");
	for (auto i : allCacheFiles)
	{
		HString name = i.baseName;
		auto split = name.Split("@");
		if (split[0].IsSame(ShaderName))
		{
			if (
					(split[1].IsSame("vs") && type == ShaderType::VertexShader)
				||	(split[1].IsSame("ps") && type == ShaderType::PixelShader)
				||	(split[1].IsSame("cs") && type == ShaderType::ComputeShader)
				)
			{
				Shader::LoadShaderCacheFile(i.absPath.c_str());
			}
		}
	}
}

void Shader::LoadShaderCacheFile(const char* cacheFilePath)
{
	HString path = cacheFilePath;
	path = FileSystem::GetFilePath(path);
	auto allCacheFiles = FileSystem::GetFilesBySuffix(path.c_str(), "spv");
	bool bSucceed = false;
	for (auto i : allCacheFiles)
	{
		if (i.absPath != cacheFilePath)
			continue;
		bSucceed = true;
		HString fileName = i.baseName;
		auto split = fileName.Split("@");
		ShaderCache cache = {};
		//
		std::ifstream file(i.absPath.c_str(), std::ios::ate | std::ios::binary);
		size_t fileSize = static_cast<size_t>(file.tellg());
		if (fileSize <= 1)
		{
			MessageOut((HString("Load shader cache failed : ") + i.fileName + " : Small size."), false, false, "255,255,0");
			return;
		}
		file.seekg(0);
		//header
		file.read((char*)&cache.header, sizeof(ShaderCacheHeader));
		//shader varients
		for (int v = 0; v < cache.header.varientCount; v++)
		{
			ShaderVarientGroup newVarient;
			file.read((char*)&newVarient, sizeof(ShaderVarientGroup));
			cache.vi.push_back(newVarient);
		}
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
			- (sizeof(ShaderVarientGroup) * cache.header.varientCount)
			- (sizeof(ShaderParameterInfo) * cache.header.shaderParameterCount)
			- (sizeof(ShaderTextureInfo) * cache.header.shaderTextureCount)
			;
		std::vector<char> shaderData(shaderCodeSize);
		file.read(shaderData.data(), shaderCodeSize);
		VkShaderModule shaderModule;
		auto bSuccess = VulkanManager::GetManager()->CreateShaderModule(shaderData, shaderModule);
		file.close();
		if (!bSuccess)
		{
			MessageOut((HString("Load shader cache failed : ") + i.fileName), false, false, "255,255,0");
			return;
		}
		//从cache名字split[2]里获取varient bool value
		cache.varients = 0;
		if (split.size() > 2)
		{
			cache.varients = HString::ToULong(split[2]);
		}
		cache.shaderModule = shaderModule;
		cache.shaderStageInfo.module = shaderModule;
		//
		cache.shaderPath = i.relativePath;
		cache.shaderAbsPath = i.absPath;
		cache.shaderName = split[0];
		cache.shaderFullName = split[0] + "@" + split[2];
		cache.shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
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
				info->group = cache.params[i].group;
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
				info->group = cache.texs[i].group;
				info->samplerAddress = cache.texs[i].msAddress;
				info->samplerFilter = cache.texs[i].msFilter;
				cache.ti.push_back(info);
			}
		}
		//
		auto manager = VulkanManager::GetManager();
		manager->DeviceWaitIdle();
		if (split[1] == "vs")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			cache.shaderStageInfo.pName = "VSMain";
			cache.shaderType = ShaderType::VertexShader;

			std::shared_ptr<ShaderCache>newCache;
			newCache.reset(new ShaderCache);
			*newCache = cache;

			//Set new
			{
				auto vs_it = _vsShader.find(cache.shaderFullName);
				if (vs_it != _vsShader.end())
				{
					vkDestroyShaderModule(manager->GetDevice(), vs_it->second->shaderModule, VK_NULL_HANDLE);
					vs_it->second->shaderModule = VK_NULL_HANDLE;
					vs_it->second = newCache;
				}
				else
				{
					_vsShader.emplace(std::make_pair(cache.shaderFullName, newCache));
				}
			}
		}
		else if (split[1] == "ps")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			cache.shaderStageInfo.pName = "PSMain";
			cache.shaderType = ShaderType::PixelShader;

			std::shared_ptr<ShaderCache>newCache;
			newCache.reset(new ShaderCache);
			*newCache = cache;

			//Set new
			{
				auto vs_it = _psShader.find(cache.shaderFullName);
				if (vs_it != _psShader.end())
				{
					vkDestroyShaderModule(manager->GetDevice(), vs_it->second->shaderModule, VK_NULL_HANDLE);
					vs_it->second->shaderModule = VK_NULL_HANDLE;
					vs_it->second = newCache;
				}
				else
				{
					_psShader.emplace(std::make_pair(cache.shaderFullName, newCache));
				}
			}
		}
		else if (split[1] == "cs")
		{
			cache.shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			cache.shaderStageInfo.pName = "CSMain";
			cache.shaderType = ShaderType::ComputeShader;

			std::shared_ptr<ShaderCache>newCache;
			newCache.reset(new ShaderCache);
			*newCache = cache;

			//Set new
			{
				auto vs_it = _csShader.find(cache.shaderFullName);
				if (vs_it != _csShader.end())
				{
					vkDestroyShaderModule(manager->GetDevice(), vs_it->second->shaderModule, VK_NULL_HANDLE);
					vs_it->second->shaderModule = VK_NULL_HANDLE;
					vs_it->second = newCache;
				}
				else
				{
					_csShader.emplace(std::make_pair(cache.shaderFullName, newCache));
				}
			}
			
		}
	}
	if (!bSucceed)
	{
		ConsoleDebug::printf_endl(GetInternationalizationText("Renderer","A000025"), cacheFilePath);
	}
}

void Shader::ReloadMaterialShaderCacheAndPipelineObject(std::weak_ptr<Material> mat)
{
	if (mat.expired())
		return;
	HString vs_n = mat.lock()->GetPrimitive()->graphicsIndex.GetVSShaderFullName();
	HString ps_n = mat.lock()->GetPrimitive()->graphicsIndex.GetVSShaderFullName();
	std::weak_ptr<ShaderCache> vs = Shader::GetVSCache(vs_n);
	std::weak_ptr<ShaderCache> ps = Shader::GetPSCache(ps_n);
	HString vs_shaderSourceName = vs.lock()->shaderName;
	HString ps_shaderSourceName = ps.lock()->shaderName;
	HString vs_shaderSourceFileName = vs.lock()->shaderName + ".fx";
	HString ps_shaderSourceFileName = ps.lock()->shaderName + ".fx";
	HString vs_cachePath = vs.lock()->shaderAbsPath;
	HString ps_cachePath = ps.lock()->shaderAbsPath;

	auto manager = VulkanManager::GetManager();
	manager->DeviceWaitIdle();

	//删除管线
	PipelineManager::RemovePipelineObjects(mat.lock()->GetPrimitive()->graphicsIndex);

	//重新编译所有变体
#if IS_EDITOR
	vs_shaderSourceFileName = FileSystem::Append(FileSystem::GetShaderIncludeAbsPath(), vs_shaderSourceFileName);
	ps_shaderSourceFileName = FileSystem::Append(FileSystem::GetShaderIncludeAbsPath(), ps_shaderSourceFileName);
	Shaderc::ShaderCompiler::CompileShader(vs_shaderSourceFileName.c_str(), "VSMain", CompileShaderType::VertexShader);
	Shaderc::ShaderCompiler::CompileShader(ps_shaderSourceFileName.c_str(), "PSMain", CompileShaderType::PixelShader);
#endif

	//重新加载ShaderCache
	Shader::LoadShaderCacheByShaderName(FileSystem::GetShaderCacheAbsPath().c_str(), vs_shaderSourceName.c_str(), ShaderType::VertexShader);
	Shader::LoadShaderCacheByShaderName(FileSystem::GetShaderCacheAbsPath().c_str(), ps_shaderSourceName.c_str(), ShaderType::PixelShader);
	//需要重新加载对应材质
	auto allMaterials = ContentManager::Get()->GetAssets(AssetType::Material);
	for (auto& i : allMaterials)
	{
		auto matCache = i.second->GetAssetObject<Material>();
		if (
			matCache.lock()->GetPrimitive()->graphicsIndex.GetVSShaderName() == vs_shaderSourceName
			|| matCache.lock()->GetPrimitive()->graphicsIndex.GetPSShaderName() == ps_shaderSourceName
			)
		{
			i.second->NeedToReload();
			Material::LoadAsset(i.second->guid);
		}
	}
}

void Shader::DestroyAllShaderCache()
{
	auto manager = VulkanManager::GetManager();
	vkDeviceWaitIdle(manager->GetDevice());
	for (auto& i : _vsShader)
	{
		vkDestroyShaderModule(manager->GetDevice(), i.second->shaderModule, VK_NULL_HANDLE);
	}
	for (auto& i : _psShader)
	{
		vkDestroyShaderModule(manager->GetDevice(), i.second->shaderModule, VK_NULL_HANDLE);
	}
	for (auto& i : _csShader)
	{
		vkDestroyShaderModule(manager->GetDevice(), i.second->shaderModule, VK_NULL_HANDLE);
	}
	_vsShader.clear();
	_psShader.clear();
	_csShader.clear();
}
