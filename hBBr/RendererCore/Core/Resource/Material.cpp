#include "Material.h"
#include "Shader.h"
#include "VertexFactory.h"
#include "VulkanManager.h"
#include "FileSystem.h"
#include "RendererType.h"
#include "ContentManager.h"

static const char* DefaultMaterial = 
"< ? xml version = \"1.0\" encoding = \"UTF - 8\" ? >"
"<root GUID = \"7117EDF7-FECB-45C3-AB7A-A4DD4A32FFAE\" >"
"<MaterialPrimitive vsShader = \"PBR\" psShader = \"PBR\" pass = \"0\" / >"
"</root>"
;

Material::Material()
{
}

Material::~Material()
{
	PrimitiveProxy::RemoveMaterialPrimitive(_primitive->passUsing, _primitive.get());
}

std::weak_ptr<Material> Material::LoadMaterial(HGUID guid)
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
	auto dataPtr = reinterpret_cast<AssetInfo<Material>*>(it->second);
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	//获取实际路径
	HString filePath = FileSystem::GetProgramPath() + it->second->relativePath + guidStr + ".mat";
	FileSystem::CorrectionPath(filePath);
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return std::weak_ptr<Material>();
	}

	pugi::xml_document materialDoc;
	HString vsFullName;
	HString psFullName;
	if (XMLStream::LoadXML(filePath.c_wstr(), materialDoc))
	{
		std::shared_ptr<Material> mat (new Material) ;
		auto root = materialDoc.child(L"root");
		auto materialPrim = root.child(L"MaterialPrimitive");
		//MaterialPrimitive
		mat->_primitive.reset(new MaterialPrimitive());
		mat->_assetInfo = dataPtr;
		mat->_primitive->graphicsName = it->second->name;
		XMLStream::LoadXMLAttributeString(materialPrim, L"vsShader", mat->_primitive->vsShader);
		XMLStream::LoadXMLAttributeString(materialPrim, L"psShader", mat->_primitive->psShader);
		uint32_t vs_varient = 0;
		uint32_t ps_varient = 0;
		XMLStream::LoadXMLAttributeUInt(materialPrim, L"vsVarient", vs_varient);
		XMLStream::LoadXMLAttributeUInt(materialPrim, L"psVarient", ps_varient);
		mat->_primitive->graphicsIndex.SetVarient(vs_varient, ps_varient);
		uint32_t pass;
		XMLStream::LoadXMLAttributeUInt(materialPrim, L"pass", pass);
		mat->_primitive->passUsing = (Pass)pass;

		vsFullName = mat->_primitive->vsShader + "@" + HString::FromUInt(mat->_primitive->graphicsIndex.GetVSVarient());
		psFullName = mat->_primitive->psShader + "@" + HString::FromUInt(mat->_primitive->graphicsIndex.GetPSVarient());
		auto vsCache = Shader::_vsShader[vsFullName];
		auto psCache = Shader::_psShader[psFullName];
		mat->_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(vsCache.header.vertexInput);
		//Parameters
		{
			auto parameters = root.child(L"Parameters");
			//primitive参数的长度以shader为主
			mat->_primitive->_paramterInfos.resize(psCache.header.shaderParameterCount);
			mat->_primitive->uniformBuffer.reserve(psCache.header.shaderParameterCount);
			//初始化
			glm::vec4 param = glm::vec4(0);
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
			param = glm::vec4(0);
			int alignmentFloat4 = 0; // float4 对齐
			uint32_t arrayIndex = 0;
			int paramIndex = 0;
			for (auto i = parameters.first_child(); i != NULL; i = i.next_sibling())
			{
				int type;
				XMLStream::LoadXMLAttributeInt(i, L"type", type);
				HString matParamName;
				XMLStream::LoadXMLAttributeString(i, L"name", matParamName);
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
					XMLStream::LoadXMLAttributeString(i, L"ui", info.ui);
					//value
					HString value;
					XMLStream::LoadXMLAttributeString(i, L"value", value);
					auto splitValue = value.Split(",");
					if (alignmentFloat4 + splitValue.size() > 4)//超出部分直接进行填充
					{
						mat->_primitive->uniformBuffer[arrayIndex] = (param);
						alignmentFloat4 = 0;
						param = glm::vec4(0);
						info.arrayIndex = arrayIndex;
						arrayIndex++;
					}
					info.vec4Index = alignmentFloat4;
					for (int i = 0; i < splitValue.size(); i++)
					{
						param[alignmentFloat4] = (float)HString::ToDouble(splitValue[i]);
						alignmentFloat4++;
						if (alignmentFloat4 >= 4)
						{
							mat->_primitive->uniformBuffer[arrayIndex] = (param);
							alignmentFloat4 = 0;
							param = glm::vec4(0);
							info.arrayIndex = arrayIndex;
							arrayIndex++;
						}
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
			auto textures = root.child(L"Textures");
			int texCount = 0;
			//primitive参数的长度以shader为主
			mat->_primitive->_textureInfos.reserve(psCache.header.shaderTextureCount);
			mat->_primitive->textures.resize(psCache.header.shaderTextureCount);
			mat->_primitive->_samplers.resize(psCache.header.shaderTextureCount);
			//初始化
			for (int i = 0; i < psCache.header.shaderTextureCount; i++)
			{
				mat->_primitive->_textureInfos.push_back(*psCache.ti[i]);
				mat->_primitive->SetTexture(i, Texture::GetSystemTexture(psCache.texs[i].defaultTexture));
				//Set Sampler
				if (psCache.texs[i].msFilter == MSFilter::Nearest)
				{
					if (psCache.texs[i].msAddress == MSAddress::Clamp)
						mat->_primitive->SetTextureSampler(i, Texture::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp));
					else if (psCache.texs[i].msAddress == MSAddress::Wrap)
						mat->_primitive->SetTextureSampler(i, Texture::GetSampler(TextureSampler::TextureSampler_Nearest_Wrap));
					else if (psCache.texs[i].msAddress == MSAddress::Mirror)
						mat->_primitive->SetTextureSampler(i, Texture::GetSampler(TextureSampler::TextureSampler_Nearest_Mirror));
					else if (psCache.texs[i].msAddress == MSAddress::Border)
						mat->_primitive->SetTextureSampler(i, Texture::GetSampler(TextureSampler::TextureSampler_Nearest_Border));
				}
				else if (psCache.texs[i].msFilter == MSFilter::Linear)
				{
					if (psCache.texs[i].msAddress == MSAddress::Clamp)
						mat->_primitive->SetTextureSampler(i, Texture::GetSampler(TextureSampler::TextureSampler_Linear_Clamp));
					else if (psCache.texs[i].msAddress == MSAddress::Wrap)
						mat->_primitive->SetTextureSampler(i, Texture::GetSampler(TextureSampler::TextureSampler_Linear_Wrap));
					else if (psCache.texs[i].msAddress == MSAddress::Mirror)
						mat->_primitive->SetTextureSampler(i, Texture::GetSampler(TextureSampler::TextureSampler_Linear_Mirror));
					else if (psCache.texs[i].msAddress == MSAddress::Border)
						mat->_primitive->SetTextureSampler(i, Texture::GetSampler(TextureSampler::TextureSampler_Linear_Border));
				}
			}
			//赋值
			for (auto i = textures.first_child(); i != NULL; i = i.next_sibling())
			{
				int type;
				XMLStream::LoadXMLAttributeInt(i, L"type", type);
				HString matParamName;
				XMLStream::LoadXMLAttributeString(i, L"name", matParamName);
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
					XMLStream::LoadXMLAttributeString(i, L"ui", info.ui);
					//value
					HString value;
					XMLStream::LoadXMLAttributeString(i, L"value", value);
					HGUID guid;
					StringToGUID(value.c_str(), &guid);
					if (!guid.isValid())
					{
						MessageOut(HString(("Material load texture failed : ") + value).c_str(), false, false, "255,0,0");
						Texture::GetSystemTexture(it->defaultTexture);
						mat->_primitive->SetTexture(it->index, Texture::GetSystemTexture(it->defaultTexture));
					}
					else
					{
						if (info.type == MTType::Texture2D)
						{
							auto asset = ContentManager::Get()->GetAsset<Texture>(guid, AssetType::Texture2D);
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

Material* Material::CreateMaterial(HString newMatFilePath)
{
	if (!FileSystem::IsDir(newMatFilePath.c_str()))
	{
		return NULL;
	}
	//复制引擎自带材质实例
	HGUID guid("61A147FF-32BD-48EC-B523-57BC75EB16BA");
	HString srcMat = (FileSystem::GetContentAbsPath() + "Core/Material/61A147FF-32BD-48EC-B523-57BC75EB16BA.mat");
	FileSystem::FileCopy(srcMat.c_str() ,newMatFilePath.c_str());
	HString newName = (newMatFilePath + "/NewMaterial.mat");
	FileSystem::FileRename(srcMat.c_str(), newName.c_str());
	auto assetInfo = ContentManager::Get()->ImportAssetInfo(AssetType::Material, newName.c_str(), newMatFilePath);
	return NULL;
}