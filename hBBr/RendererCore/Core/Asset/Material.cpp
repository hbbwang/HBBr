#include "Material.h"
#include "Shader.h"
#include "VertexFactory.h"
#include "VulkanManager.h"
#include "FileSystem.h"
#include "RendererType.h"
#include "ContentManager.h"
#include "RendererConfig.h"
#include "ConsoleDebug.h"
#include "Texture2D.h"
#include "TextureCube.h"

Material::Material()
{
}

Material::~Material()
{
	PrimitiveProxy::RemoveMaterialPrimitive(_primitive->_passUsing, _primitive.get());
}

std::weak_ptr<Material> Material::GetDefaultMaterial()
{
	static std::weak_ptr<Material> defaultMat;
	if (defaultMat.expired())
	{
		defaultMat = Material::LoadAsset(HGUID("b51e2e9a-0985-75e8-6138-fa95efcbab57"));
	}
	return defaultMat;
}

std::weak_ptr<Material> Material::GetErrorMaterial()
{
	static std::weak_ptr<Material> errorMat;
	if (errorMat.expired())
	{
		errorMat = Material::LoadAsset(HGUID("22d44cd6-68c3-4997-ad5f-3d52c45ef8fe"));
	}
	return errorMat;
}

std::weak_ptr<Material> Material::LoadAsset(HGUID guid)
{
	const auto matAssets = ContentManager::Get()->GetAssets(AssetType::Material);
	HString guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = matAssets.find(guid);
	{
		if (it == matAssets.end())
		{
			MessageOut(HString("Can not find [" + guidStr + "] material in content manager."), false, false, "255,255,0");
			return std::weak_ptr<Material>();
		}
	}
	auto dataPtr = std::static_pointer_cast<AssetInfo<Material>>(it->second);

	//是否需要重新加载
	bool bReload = false;
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	else if (!dataPtr->IsAssetLoad() && dataPtr->GetMetadata())
	{
		bReload = true;
	}

	//获取实际路径
	HString filePath = it->second->absFilePath;
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return std::weak_ptr<Material>();
	}

	nlohmann::json json;
	HString vsFullName;
	HString psFullName;
	if (Serializable::LoadJson(filePath, json))
	{
		std::shared_ptr<Material> mat;
		if (!bReload)
		{
			mat.reset(new Material);
			mat->_primitive.reset(new MaterialPrimitive());
		}
		else
		{
			//重新刷新asset
			mat = dataPtr->GetMetadata();
		}
		//MaterialPrimitive
		mat->_assetInfo = dataPtr;
		mat->_primitive->_graphicsName = it->second->displayName;

		uint32_t vs_varient = 0;
		uint32_t ps_varient = 0;
		vs_varient = json["vsVarient"];
		ps_varient = json["psVarient"];
		HString vsShaderName = json["vsShader"];
		HString psShaderName = json["psShader"];
		vsFullName = vsShaderName + "@" + HString::FromUInt(vs_varient);
		psFullName = psShaderName + "@" + HString::FromUInt(ps_varient);

		//根据变体查找ShaderCache
		std::shared_ptr<ShaderCache> vsCache;
		std::shared_ptr<ShaderCache> psCache;
		{
			auto vs_it = Shader::_vsShader.find(vsFullName);
			if (vs_it != Shader::_vsShader.end())
				vsCache = vs_it->second;
			else
			{
				ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000023"), vsFullName.c_str());
				vsCache = Shader::_vsShader["Error@0"];
			}
		}
		{
			auto ps_it = Shader::_psShader.find(psFullName);
			if (ps_it != Shader::_psShader.end())
				psCache = ps_it->second;
			else
			{
				ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000023"), psFullName.c_str());
				psCache = Shader::_psShader["Error@0"];
			}
		}

		//获取布局
		mat->_primitive->_inputLayout = VertexFactory::VertexInput::BuildLayout(vsCache->header.vertexInput);

		//Parameters VS
		{
			//primitive参数的长度以shader为主
			//初始化
			//重载模式,读取老参数
			if (bReload && mat->_primitive->_uniformBuffer_vs.size() > 0)
			{
				//缓存老参数
				auto old_vs_ub_Info = mat->_primitive->_paramterInfos_vs;
				auto old_vs_ub = mat->_primitive->_uniformBuffer_vs;
				mat->_primitive->_paramterInfos_vs.resize(vsCache->header.shaderParameterCount_vs);
				mat->_primitive->_uniformBuffer_vs.resize(vsCache->header.shaderParameterCount_vs);
				for (int i = 0; i < vsCache->header.shaderParameterCount_vs; i++)
				{
					mat->_primitive->_paramterInfos_vs[i] = *vsCache->pi_vs[i];
					auto oldMatParamIt = std::find_if(old_vs_ub_Info.begin(), old_vs_ub_Info.end(), [&](MaterialParameterInfo& oldMatParam) {
						return oldMatParam.name.IsSame(vsCache->pi_vs[i]->name) && oldMatParam.type == vsCache->pi_vs[i]->type;
					});
					if (oldMatParamIt != old_vs_ub_Info.end())
					{
						if(oldMatParamIt->type == MPType::VSFloat)
						{
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index];
						}
						else if (oldMatParamIt->type == MPType::VSFloat2)
						{
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index];
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index+1] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index+1];
						}
						else if (oldMatParamIt->type == MPType::VSFloat3)
						{
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index];
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index + 1] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 1];
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index + 2] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 2];
						}
						else if (oldMatParamIt->type == MPType::VSFloat4)
						{
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index];
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index + 1] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 1];
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index + 2] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 2];
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index + 3] = old_vs_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 3];
						}
					}
					else
					{
						for (int p = 0; p < vsCache->pi_vs[i]->value.size(); p++)
						{
							mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index + p] = vsCache->pi_vs[i]->value[p];
						}
					}
				}
				auto anlignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(glm::vec4) * mat->_primitive->_uniformBuffer_vs.size());
				mat->_primitive->_uniformBufferSize_vs = anlignmentSize;
			}
			else
			{
				mat->_primitive->_paramterInfos_vs.resize(vsCache->header.shaderParameterCount_vs);
				mat->_primitive->_uniformBuffer_vs.resize(vsCache->header.shaderParameterCount_vs);
				for (int i = 0; i < vsCache->header.shaderParameterCount_vs; i++)
				{
					mat->_primitive->_paramterInfos_vs[i] = *vsCache->pi_vs[i];
					for (int p = 0; p < vsCache->pi_vs[i]->value.size(); p++)
					{
						mat->_primitive->_uniformBuffer_vs[vsCache->pi_vs[i]->arrayIndex][vsCache->pi_vs[i]->vec4Index + p] = vsCache->pi_vs[i]->value[p];
					}
				}
				auto anlignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(glm::vec4) * mat->_primitive->_uniformBuffer_vs.size());
				mat->_primitive->_uniformBufferSize_vs = anlignmentSize;
				auto params_it = json.find("parameters_vs");
				if (params_it != json.end())
				{
					int paramIndex = 0;
					for (auto& i : params_it.value().items())
					{
						HString matParamName = i.key();
						int type = i.value()["type"];
						//从shader中查找是否存在相同参数(名字&类型)
						auto it = std::find_if(mat->_primitive->_paramterInfos_vs.begin(), mat->_primitive->_paramterInfos_vs.end(), [type, matParamName](MaterialParameterInfo& info) {
							return matParamName == info.name && type == (int)info.type;
							});
						if (it != mat->_primitive->_paramterInfos_vs.end())
						{
							auto& info = *it;
							//value
							std::vector<float> value;
							if (info.type == MPType::VSFloat)
							{
								float vf = 0;
								from_json(i.value()["value"], vf);
								value.push_back(vf);
							}
							else if (info.type == MPType::VSFloat2)
							{
								glm::vec2 v2;
								from_json(i.value()["value"], v2);
								value.reserve(2);
								value.push_back(v2.x);
								value.push_back(v2.y);
							}
							else if (info.type == MPType::VSFloat3)
							{
								glm::vec3 v3;
								from_json(i.value()["value"], v3);
								value.reserve(3);
								value.push_back(v3.x);
								value.push_back(v3.y);
								value.push_back(v3.z);
							}
							else if (info.type == MPType::VSFloat4)
							{
								glm::vec4 v4;
								from_json(i.value()["value"], v4);
								value.reserve(4);
								value.push_back(v4.x);
								value.push_back(v4.y);
								value.push_back(v4.z);
								value.push_back(v4.w);
							}

							for (uint32_t pi = 0; pi < info.value.size(); pi++)
							{
								mat->_primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index + pi] = value[pi];
							}
							paramIndex++;
						}
					}
				}
			}
		}

		//Parameters PS
		{
			//primitive参数的长度以shader为主
			//初始化
			//重载模式,读取老参数
			if (bReload && mat->_primitive->_uniformBuffer_ps.size() > 0)
			{				
				//缓存老参数
				auto old_ps_ub_Info = mat->_primitive->_paramterInfos_ps;
				auto old_ps_ub = mat->_primitive->_uniformBuffer_ps;
				mat->_primitive->_paramterInfos_ps.resize(psCache->header.shaderParameterCount_ps);
				mat->_primitive->_uniformBuffer_ps.resize(psCache->header.shaderParameterCount_ps);
				for (int i = 0; i < psCache->header.shaderParameterCount_ps; i++)
				{
					mat->_primitive->_paramterInfos_ps[i] = *psCache->pi_ps[i];
					auto oldMatParamIt = std::find_if(old_ps_ub_Info.begin(), old_ps_ub_Info.end(), [&](MaterialParameterInfo& oldMatParam) {
						return oldMatParam.name.IsSame(psCache->pi_ps[i]->name) && oldMatParam.type == psCache->pi_ps[i]->type;
						});
					if (oldMatParamIt != old_ps_ub_Info.end())
					{
						if (oldMatParamIt->type == MPType::PSFloat)
						{
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index];
						}
						else if (oldMatParamIt->type == MPType::PSFloat2)
						{
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index];
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index + 1] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 1];
						}
						else if (oldMatParamIt->type == MPType::PSFloat3)
						{
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index];
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index + 1] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 1];
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index + 2] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 2];
						}
						else if (oldMatParamIt->type == MPType::PSFloat4)
						{
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index];
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index + 1] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 1];
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index + 2] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 2];
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index + 3] = old_ps_ub[oldMatParamIt->arrayIndex][oldMatParamIt->vec4Index + 3];
						}
					}
					else
					{
						for (int p = 0; p < psCache->pi_ps[i]->value.size(); p++)
						{
							mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index + p] = psCache->pi_ps[i]->value[p];
						}
					}
				}
				auto anlignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(glm::vec4) * mat->_primitive->_uniformBuffer_ps.size());
				mat->_primitive->_uniformBufferSize_ps = anlignmentSize;
			}
			else
			{
				mat->_primitive->_paramterInfos_ps.resize(psCache->header.shaderParameterCount_ps);
				mat->_primitive->_uniformBuffer_ps.resize(psCache->header.shaderParameterCount_ps);
				for (int i = 0; i < psCache->header.shaderParameterCount_ps; i++)
				{
					mat->_primitive->_paramterInfos_ps[i] = *psCache->pi_ps[i];
					for (int p = 0; p < psCache->pi_ps[i]->value.size(); p++)
					{
						mat->_primitive->_uniformBuffer_ps[psCache->pi_ps[i]->arrayIndex][psCache->pi_ps[i]->vec4Index + p] = psCache->pi_ps[i]->value[p];
					}
				}
				auto anlignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(glm::vec4) * mat->_primitive->_uniformBuffer_ps.size());
				mat->_primitive->_uniformBufferSize_ps = anlignmentSize;
				auto params_it = json.find("parameters_ps");
				if (params_it != json.end())
				{
					int paramIndex = 0;
					for (auto& i : params_it.value().items())
					{
						HString matParamName = i.key();
						int type = i.value()["type"];
						//从shader中查找是否存在相同参数(名字&类型)
						auto it = std::find_if(mat->_primitive->_paramterInfos_ps.begin(), mat->_primitive->_paramterInfos_ps.end(), [type, matParamName](MaterialParameterInfo& info) {
							return matParamName == info.name && type == (int)info.type;
							});
						if (it != mat->_primitive->_paramterInfos_ps.end())
						{
							auto& info = *it;
							//value
							std::vector<float> value;
							if (info.type == MPType::PSFloat)
							{
								float vf = 0;
								from_json(i.value()["value"], vf);
								value.push_back(vf);
							}
							else if (info.type == MPType::PSFloat2)
							{
								glm::vec2 v2;
								from_json(i.value()["value"], v2);
								value.reserve(2);
								value.push_back(v2.x);
								value.push_back(v2.y);
							}
							else if (info.type == MPType::PSFloat3)
							{
								glm::vec3 v3;
								from_json(i.value()["value"], v3);
								value.reserve(3);
								value.push_back(v3.x);
								value.push_back(v3.y);
								value.push_back(v3.z);
							}
							else if (info.type == MPType::PSFloat4)
							{
								glm::vec4 v4;
								from_json(i.value()["value"], v4);
								value.reserve(4);
								value.push_back(v4.x);
								value.push_back(v4.y);
								value.push_back(v4.z);
								value.push_back(v4.w);
							}

							for (uint32_t pi = 0; pi < info.value.size(); pi++)
							{
								mat->_primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index + pi] = value[pi];
							}
							paramIndex++;
						}
					}
				}
			}
		}

		//Textures
		{
			auto old_texs_info = mat->_primitive->_textureInfos;
			auto old_texs = mat->_primitive->_textures;
			auto old_samplers = mat->_primitive->_samplers;
			{
				//primitive参数的长度以shader为主
				//初始化
				mat->_primitive->_textureInfos.resize(psCache->header.shaderTextureCount);
				mat->_primitive->_textures.resize(psCache->header.shaderTextureCount);
				mat->_primitive->_samplers.resize(psCache->header.shaderTextureCount);
				//初始化
				for (int i = 0; i < psCache->header.shaderTextureCount; i++)
				{
					//Set textures
					//查找刷新之后，新的shader里，是否有刷新之前的材质绑定的纹理信息，有就直接用
					auto oldMatParamIt = std::find_if(old_texs_info.begin(), old_texs_info.end(), [&](MaterialTextureInfo& oldMatParam) {
							return oldMatParam.name.IsSame(psCache->pi_ps[i]->name) && oldMatParam.type == psCache->ti[i]->type;
						});
					if (oldMatParamIt != old_texs_info.end())
					{
						mat->_primitive->SetTexture(i, old_texs[oldMatParamIt->index]);
						mat->_primitive->SetTextureSampler(i, old_samplers[oldMatParamIt->index]);
					}
					else
					{
						mat->_primitive->_textureInfos[i] = (*psCache->ti[i]);
						auto defaultTex = Texture2D::GetSystemTexture(psCache->texs[i].defaultTexture);
						if (mat->_primitive->_textureInfos[i].type == MTType::TextureCube)
						{
							if (!defaultTex->_imageData.isCubeMap)
							{
								mat->_primitive->_textureInfos[i].value = "CubeMapBalck";
								defaultTex = Texture2D::GetSystemTexture("CubeMapBalck");
							}
						}
						mat->_primitive->SetTexture(i, defaultTex);
						//Set Sampler
						if (psCache->texs[i].msFilter == MSFilter::Nearest)
						{
							if (psCache->texs[i].msAddress == MSAddress::Clamp)
								mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp));
							else if (psCache->texs[i].msAddress == MSAddress::Wrap)
								mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Wrap));
							else if (psCache->texs[i].msAddress == MSAddress::Mirror)
								mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Mirror));
							else if (psCache->texs[i].msAddress == MSAddress::Border)
								mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Border));
						}
						else if (psCache->texs[i].msFilter == MSFilter::Linear)
						{
							if (psCache->texs[i].msAddress == MSAddress::Clamp)
								mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Clamp));
							else if (psCache->texs[i].msAddress == MSAddress::Wrap)
								mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Wrap));
							else if (psCache->texs[i].msAddress == MSAddress::Mirror)
								mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Mirror));
							else if (psCache->texs[i].msAddress == MSAddress::Border)
								mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Border));
						}
					}
				}
			}
			auto texs_it = json.find("textures");
			if (texs_it != json.end())
			{
				int texCount = 0;
				//赋值
				for (auto& i : texs_it.value().items())
				{
					int type = i.value()["type"];
					HString matParamName = i.key();
					//从shader中查找是否存在相同参数(名字&类型)
					auto it = std::find_if(mat->_primitive->_textureInfos.begin(), mat->_primitive->_textureInfos.end(), [type, matParamName](MaterialTextureInfo& info) {
						return matParamName == info.name && type == (int)info.type;
						});
					if (it != mat->_primitive->_textureInfos.end())
					{
						auto& info = *it;
						//value
						HString value = i.value()["value"];
						HGUID guid;
						StringToGUID(value.c_str(), &guid);
						if (!guid.isValid())
						{
							//MessageOut((GetInternationalizationText("Renderer","A000020") + value), false, false, "255,0,0");
							auto tex = Texture2D::GetSystemTexture(info.value);
							if (info.type == MTType::Texture2D && tex->_imageData.isCubeMap)
							{
								tex = Texture2D::GetSystemTexture("Black");
							}
							else if ((info.type == MTType::TextureCube && !tex->_imageData.isCubeMap) || (info.type == MTType::TextureCube && info.value.IsSame("Black", false)))
							{
								tex = Texture2D::GetSystemTexture("CubeMapBalck");
							}

							mat->_primitive->SetTexture(it->index, tex);
						}
						else
						{
							if (info.type == MTType::Texture2D)
							{
								auto asset = ContentManager::Get()->GetAsset<Texture2D>(guid, AssetType::Texture2D);
								if (!asset.expired())
									mat->_primitive->SetTexture(it->index, asset.lock().get());
							}
							else if (info.type == MTType::TextureCube)
							{
								auto asset = ContentManager::Get()->GetAsset<TextureCube>(guid, AssetType::Texture2D);
								if (!asset.expired())
									mat->_primitive->SetTexture(it->index, asset.lock().get());
							}
						}
						texCount++;
					}
				}
			}
		}

		//创建Material Primitive
		PrimitiveProxy::GetNewMaterialPrimitiveIndex(mat->_primitive.get(), vsCache, psCache);

		//Blend mode
		{
			if (!bReload)
			{
				auto bm_it = json.find("blendMode");
				if (bm_it != json.end())
				{
					mat->_primitive->_graphicsIndex.blendMode = (BlendMode)bm_it.value();
				}
			}

			//Pass分类
			if (mat->_primitive->_graphicsIndex.blendMode == BlendMode::Opaque)
			{
				mat->_primitive->_passUsing = Pass::OpaquePass;
			}

		}

		if (!bReload)
		{
			PrimitiveProxy::AddMaterialPrimitive(mat->_primitive.get());
		}

		dataPtr->SetData(std::move(mat));
		//_allMaterials.emplace(std::make_pair(guid, std::move(mat)));
		return dataPtr->GetData();
	}
	return std::weak_ptr<Material>();
}

void Material::SaveAsset(HString path)
{
	ToJson();
	SaveJson(path);
	ContentManager::Get()->SaveAssetInfo(this->_assetInfo);
}

std::weak_ptr<AssetInfoBase> Material::CreateMaterial(HString repository, HString virtualPath)
{
	HString repositoryPath = FileSystem::GetRepositoryAbsPath(repository);
	//生成材质xml文件
	HString savePath = FileSystem::Append(repositoryPath, "Material");
	HString saveFilePath = FileSystem::Append(savePath, "NewMaterial");
	int index = -1;
	while (true)
	{
		if (FileSystem::FileExist(saveFilePath))
		{
			index++;
			saveFilePath = FileSystem::Append(savePath, HString("NewMaterial_") + HString::FromInt(index));
		}
		else
		{
			break;
		}
	}
	nlohmann::json json;
	saveFilePath += ".mat";

	json["vsShader"] = "PBR";
	json["psShader"] = "PBR";
	json["vsVarient"] = 0;
	json["psVarient"] = 0;
	json["blendMode"] = 0;

	if (Serializable::SaveJson(json, saveFilePath.c_str()))
	{
		std::vector<std::weak_ptr<AssetInfoBase> > results;
		//导入AssetInfo
		AssetImportInfo newInfo;
		newInfo.absAssetFilePath = saveFilePath;
		newInfo.virtualPath = virtualPath;
		ContentManager::Get()->AssetImport(repository, { newInfo }, &results);
		MessageOut(GetInternationalizationText("Renderer", "A000021"), false, false, "0,255,0");
		return results[0];
	}
	else
	{
		MessageOut(GetInternationalizationText("Renderer", "A000022"), false, false, "255,0,0");
	}
	return std::weak_ptr<AssetInfoBase>();
}

nlohmann::json Material::ToJson()
{
	//Base
	_json["vsShader"] = GetPrimitive()->_graphicsIndex.GetVSShaderName();
	_json["psShader"] = GetPrimitive()->_graphicsIndex.GetPSShaderName();
	_json["blendMode"] = (int)GetPrimitive()->_graphicsIndex.blendMode;
	_json["vsVarient"] = GetPrimitive()->_graphicsIndex.vs_varients;
	_json["psVarient"] = GetPrimitive()->_graphicsIndex.ps_varients;
	//Parameters PS
	{
		nlohmann::json p;
		for (int i = 0; i < _primitive->_paramterInfos_ps.size(); i++)
		{
			auto& info = _primitive->_paramterInfos_ps[i];
			if (info.type == MPType::PSFloat)
			{
				p[info.name.c_str()]["value"] = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index];
			}
			else if (info.type == MPType::PSFloat2)
			{
				glm::vec2 v2;
				v2.x = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index];
				v2.y = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index+1];
				to_json(p[info.name.c_str()]["value"], v2);
			}
			else if (info.type == MPType::PSFloat3)
			{
				glm::vec3 v3;
				v3.x = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index];
				v3.y = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index + 1];
				v3.z = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index + 2];
				to_json(p[info.name.c_str()]["value"], v3);
			}
			else if (info.type == MPType::PSFloat4)
			{
				glm::vec4 v4;
				v4.x = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index];
				v4.y = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index + 1];
				v4.z = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index + 2];
				v4.w = _primitive->_uniformBuffer_ps[info.arrayIndex][info.vec4Index + 3];
				to_json(p[info.name.c_str()]["value"], v4);
			}
			p[info.name.c_str()]["type"] = (int)_primitive->_paramterInfos_ps[i].type;
		}
		_json["parameters_ps"] = p;
	}

	//Parameters VS
	{
		nlohmann::json p;
		for (int i = 0; i < _primitive->_paramterInfos_vs.size(); i++)
		{
			auto& info = _primitive->_paramterInfos_vs[i];
			if (info.type == MPType::VSFloat)
			{
				p[info.name.c_str()]["value"] = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index];
			}
			else if (info.type == MPType::VSFloat2)
			{
				glm::vec2 v2;
				v2.x = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index];
				v2.y = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index + 1];
				to_json(p[info.name.c_str()]["value"], v2);
			}
			else if (info.type == MPType::VSFloat3)
			{
				glm::vec3 v3;
				v3.x = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index];
				v3.y = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index + 1];
				v3.z = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index + 2];
				to_json(p[info.name.c_str()]["value"], v3);
			}
			else if (info.type == MPType::VSFloat4)
			{
				glm::vec4 v4;
				v4.x = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index];
				v4.y = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index + 1];
				v4.z = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index + 2];
				v4.w = _primitive->_uniformBuffer_vs[info.arrayIndex][info.vec4Index + 3];
				to_json(p[info.name.c_str()]["value"], v4);
			}
			p[info.name.c_str()]["type"] = (int)_primitive->_paramterInfos_vs[i].type;
		}
		_json["parameters_vs"] = p;
	}

	//Textures
	{
		nlohmann::json t;
		for (int i = 0; i < _primitive->_textureInfos.size(); i++)
		{
			if (_primitive->_textures.size() > i)
			{
				t[_primitive->_textureInfos[i].name.c_str()]["value"] = _primitive->_textures[i]->_assetInfo.lock()->guid;
				//考虑到需要区分texture2d和textureCube，所以还是要有个type
				t[_primitive->_textureInfos[i].name.c_str()]["type"] = (int)_primitive->_textures[i]->_assetInfo.lock()->type;
			}
		}
		_json["textures"] = t;
	}

	return _json;
}

void Material::FromJson()
{


}