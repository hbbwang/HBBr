#include "Material.h"
#include "Shader.h"
#include "VertexFactory.h"
#include "VulkanManager.h"
#include "FileSystem.h"
#include "RendererType.h"
#include "ContentManager.h"
#include "Asset/Serializable.h"

Material::Material()
{
}

Material::~Material()
{
	PrimitiveProxy::RemoveMaterialPrimitive(_primitive->passUsing, _primitive.get());
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
			MessageOut(HString("Can not find [" + guidStr + "] material in content manager.").c_str(), false, false, "255,255,0");
			return std::weak_ptr<Material>();
		}
	}
	auto dataPtr = std::static_pointer_cast<AssetInfo<Material>>(it->second);
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
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
		std::shared_ptr<Material> mat (new Material) ;
		//MaterialPrimitive
		mat->_primitive.reset(new MaterialPrimitive());
		mat->_assetInfo = dataPtr.get();
		mat->_primitive->graphicsName = it->second->displayName;
		mat->_primitive->vsShader = json["vsShader"];
		mat->_primitive->psShader = json["psShader"];
		uint32_t vs_varient = 0;
		uint32_t ps_varient = 0;
		vs_varient = json["vsVarient"];
		ps_varient = json["psVarient"];
		mat->_primitive->graphicsIndex.SetVarient(vs_varient, ps_varient);
		uint32_t pass = json["pass"];

		mat->_primitive->passUsing = (Pass)pass;

		vsFullName = mat->_primitive->vsShader + "@" + HString::FromUInt(mat->_primitive->graphicsIndex.GetVSVarient());
		psFullName = mat->_primitive->psShader + "@" + HString::FromUInt(mat->_primitive->graphicsIndex.GetPSVarient());
		auto vsCache = Shader::_vsShader[vsFullName];
		auto psCache = Shader::_psShader[psFullName];
		mat->_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(vsCache.header.vertexInput);

		//Parameters
		{
			nlohmann::json parameters = json["Parameters"];
			//primitive参数的长度以shader为主
			mat->_primitive->_paramterInfos.resize(psCache.header.shaderParameterCount);
			mat->_primitive->uniformBuffer.reserve(psCache.header.shaderParameterCount);
			//初始化
			for (int i = 0; i < psCache.header.shaderParameterCount; i++)
			{
				mat->_primitive->_paramterInfos[i] = *psCache.pi[i];
				for (int p = 0; p < psCache.pi[i]->value.size();p++)
				{
					if (mat->_primitive->uniformBuffer.size() <= psCache.pi[i]->arrayIndex)
					{
						mat->_primitive->uniformBuffer.push_back(glm::vec4());
					}
					mat->_primitive->uniformBuffer[psCache.pi[i]->arrayIndex][psCache.pi[i]->vec4Index + p] = psCache.pi[i]->value[p];				
				}
			}
			int paramIndex = 0;
			for (auto& i : parameters.items())
			{
				HString matParamName = i.key();
				int type = i.value()["type"];
				//从shader中查找是否存在相同参数(名字&类型)
				auto it = std::find_if(psCache.params.begin(), psCache.params.end(), [type , matParamName](ShaderParameterInfo& info) {
					return matParamName == info.name && type == (int)info.type;
				});
				if (it != psCache.params.end())
				{
					MaterialParameterInfo info = {};
					//name
					info.name = matParamName;
					//type
					info.type = (MPType)type;
					//ui
					info.ui = i.value()["ui"];
					//value
					if (info.type == MPType::Float)
					{
						info.arrayIndex;
						info.vec4Index;
					}
					else if (info.type == MPType::Float2)
					{

					}
					else if (info.type == MPType::Float3)
					{

					}
					else if (info.type == MPType::Float4)
					{

					}
					mat->_primitive->_paramterInfos[paramIndex] = (info);
					paramIndex++;
				}
			}
			auto anlignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(glm::vec4) * mat->_primitive->uniformBuffer.size());
			mat->_primitive->uniformBufferSize = anlignmentSize;
		}
		
		//Textures
		{
			nlohmann::json textures = json["Textures"];
			int texCount = 0;
			//primitive参数的长度以shader为主
			mat->_primitive->_textureInfos.reserve(psCache.header.shaderTextureCount);
			mat->_primitive->textures.resize(psCache.header.shaderTextureCount);
			mat->_primitive->_samplers.resize(psCache.header.shaderTextureCount);
			//初始化
			for (int i = 0; i < psCache.header.shaderTextureCount; i++)
			{
				mat->_primitive->_textureInfos.push_back(*psCache.ti[i]);
				mat->_primitive->SetTexture(i, Texture2D::GetSystemTexture(psCache.texs[i].defaultTexture));
				//Set Sampler
				if (psCache.texs[i].msFilter == MSFilter::Nearest)
				{
					if (psCache.texs[i].msAddress == MSAddress::Clamp)
						mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp));
					else if (psCache.texs[i].msAddress == MSAddress::Wrap)
						mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Wrap));
					else if (psCache.texs[i].msAddress == MSAddress::Mirror)
						mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Mirror));
					else if (psCache.texs[i].msAddress == MSAddress::Border)
						mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Border));
				}
				else if (psCache.texs[i].msFilter == MSFilter::Linear)
				{
					if (psCache.texs[i].msAddress == MSAddress::Clamp)
						mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Clamp));
					else if (psCache.texs[i].msAddress == MSAddress::Wrap)
						mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Wrap));
					else if (psCache.texs[i].msAddress == MSAddress::Mirror)
						mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Mirror));
					else if (psCache.texs[i].msAddress == MSAddress::Border)
						mat->_primitive->SetTextureSampler(i, Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Border));
				}
			}
			//赋值
			for (auto& i : textures.items())
			{
				int type = i.value()["type"];
				HString matParamName = i.key();
				//从shader中查找是否存在相同参数(名字&类型)
				auto it = std::find_if(psCache.texs.begin(), psCache.texs.end(), [type, matParamName](ShaderTextureInfo& info) {
					return matParamName == info.name && type == (int)info.type;
					});
				if (it != psCache.texs.end())
				{
					MaterialTextureInfo info = {};
					//name
					info.name = matParamName;
					//type
					info.type = (MTType)type;
					//index
					info.index = it->index;
					//address
					info.samplerAddress = it->msAddress;
					//filter
					info.samplerFilter = it->msFilter;
					//ui
					info.ui = i.value()["ui"];
					//value
					HString value = i.value()["value"];
					HGUID guid;
					StringToGUID(value.c_str(), &guid);
					if (!guid.isValid())
					{
						MessageOut((GetInternationalizationText("Renderer","A000020") + value).c_str(), false, false, "255,0,0");
						Texture2D::GetSystemTexture(it->defaultTexture);
						mat->_primitive->SetTexture(it->index, Texture2D::GetSystemTexture(it->defaultTexture));
					}
					else
					{
						if (info.type == MTType::Texture2D)
						{
							auto asset = ContentManager::Get()->GetAsset<Texture2D>(guid, AssetType::Texture2D);
							mat->_primitive->SetTexture(it->index, asset.lock().get());
						}					
					}
					mat->_primitive->_textureInfos.push_back(info);
				}
				else
				{

				}
			}
		}
		//
		PrimitiveProxy::GetNewMaterialPrimitiveIndex(mat->_primitive.get(), vsCache, psCache);
		PrimitiveProxy::AddMaterialPrimitive( mat->_primitive.get());

		dataPtr->SetData(std::move(mat));

		//_allMaterials.emplace(std::make_pair(guid, std::move(mat)));

		return dataPtr->GetData();
	}
	return std::weak_ptr<Material>();
}

void Material::SaveAsset(HString path)
{

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
	pugi::xml_document doc;
	saveFilePath += ".mat";

	auto root = doc.append_child(TEXT("root"));
	auto mp = XMLStream::CreateXMLNode(root,TEXT("MaterialPrimitive"));
	XMLStream::SetXMLAttribute(mp,TEXT("vsShader"),TEXT("PBR"));
	XMLStream::SetXMLAttribute(mp, TEXT("psShader"), TEXT("PBR"));
	XMLStream::SetXMLAttribute(mp, TEXT("vsVarient"), 1);
	XMLStream::SetXMLAttribute(mp, TEXT("psVarient"), 1);
	XMLStream::SetXMLAttribute(mp, TEXT("pass"), 0);

	if (XMLStream::SaveXML(saveFilePath.c_wstr(), doc))
	{
		std::vector<std::weak_ptr<AssetInfoBase> > results;
		//导入AssetInfo
		AssetImportInfo newInfo;
		newInfo.absAssetFilePath = saveFilePath;
		newInfo.virtualPath = virtualPath;
		ContentManager::Get()->AssetImport(repository, { newInfo }, &results);
		MessageOut(GetInternationalizationText("Renderer", "A000021").c_str(), false, false, "0,255,0");
		return results[0];
	}
	else
	{
		MessageOut(GetInternationalizationText("Renderer", "A000022").c_str(), false, false, "255,0,0");
	}
	return std::weak_ptr<AssetInfoBase>();
}