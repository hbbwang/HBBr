#include "Material.h"
#include "Shader.h"
#include "VertexFactory.h"
#include "VulkanManager.h"
#include "FileSystem.h"
#include "RendererType.h"
#include "ContentManager.h"

Material::Material()
{
}

Material::~Material()
{
	PrimitiveProxy::RemoveMaterialPrimitive(_primitive->passUsing, _primitive.get());
}

Material* Material::LoadMaterial(HGUID guid)
{
	const auto matAssets = ContentManager::Get()->GetAssets(AssetType::Material);
	HString guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = matAssets.find(guid);
	{
		if (it == matAssets.end())
		{
			MessageOut(HString("Can not find [" + guidStr + "] material in content manager.").c_str(), false, false, "255,255,0");
			return NULL;
		}
	}
	auto dataPtr = reinterpret_cast<AssetInfo<Material>*>(it->second);
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	//获取实际路径
	HString filePath = FileSystem::GetProgramPath() + it->second->relativePath + guidStr + ".mat";
	filePath.CorrectionPath();
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return NULL;
	}

	pugi::xml_document materialDoc;
	if (XMLStream::LoadXML(filePath.c_wstr(), materialDoc))
	{
		std::unique_ptr<Material> mat (new Material) ;
		auto root = materialDoc.child(L"root");
		if (!StringToGUID(guidStr.c_str(), &guid))
		{
			guid = CreateGUID();
			guidStr = GUIDToString(guid);
		}
		mat->_guid = guid;
		auto materialPrim = root.child(L"MaterialPrimitive");
		//MaterialPrimitive
		mat->_primitive.reset(new MaterialPrimitive());
		mat->_primitive->graphicsName = it->second->name;
		XMLStream::LoadXMLAttributeString(materialPrim, L"vsShader", mat->_primitive->vsShader);
		XMLStream::LoadXMLAttributeString(materialPrim, L"psShader", mat->_primitive->psShader);
		uint32_t pass;
		XMLStream::LoadXMLAttributeUInt(materialPrim, L"pass", pass);
		mat->_primitive->passUsing = (Pass)pass;
		auto vsCache = Shader::_vsShader[mat->_primitive->vsShader];
		auto psCache = Shader::_psShader[mat->_primitive->psShader];
		mat->_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(vsCache.header.vertexInput);
		//Parameters
		{
			auto parameters = root.child(L"Parameters");
			//primitive参数的长度以shader为主
			mat->_primitive->_paramterInfos.resize(psCache.header.shaderParameterCount);
			mat->_primitive->uniformBuffer.resize(psCache.header.shaderParameterCount);
			//初始化
			for (int i = 0; i < psCache.header.shaderTextureCount; i++)
			{
				mat->_primitive->_paramterInfos[i] = *psCache.pi[i];
				mat->_primitive->uniformBuffer[i] = psCache.pi[i]->value;
			}
			glm::vec4 param = glm::vec4(0);
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
			//初始化
			for (int i = 0; i < psCache.header.shaderTextureCount; i++)
			{
				mat->_primitive->_textureInfos.push_back(*psCache.ti[i]);
				mat->_primitive->SetTexture(i, Texture::GetSystemTexture(psCache.texs[i].defaultTexture));
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
							mat->_primitive->SetTexture(it->index, asset);
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

		PrimitiveProxy::GetNewMaterialPrimitiveIndex(mat->_primitive.get());
		PrimitiveProxy::AddMaterialPrimitive( mat->_primitive.get());

		dataPtr->SetData(std::move(mat));

		//_allMaterials.emplace(std::make_pair(guid, std::move(mat)));

		return dataPtr->GetData();
	}
	return NULL;
}

Material* Material::CreateMaterial(HString newMatFilePath)
{
	if (!FileSystem::IsNormalFile(newMatFilePath.c_str()))
	{
		return NULL;
	}
	if (!newMatFilePath.GetSuffix().IsSame("mat"))
	{
		return NULL;
	}
	//复制引擎自带材质实例
	HGUID guid("61A147FF-32BD-48EC-B523-57BC75EB16BA");
	FileSystem::FileCopy((FileSystem::GetContentAbsPath() + "Core/Material/61A147FF-32BD-48EC-B523-57BC75EB16BA.mat").c_str() ,newMatFilePath.c_str());


	return NULL;
}