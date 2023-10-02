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
	HString filePath = FileSystem::GetContentAbsPath() + it->second->relativePath + guidStr + ".mat";
	filePath.CorrectionPath();
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return NULL;
	}

	pugi::xml_document materialDoc;
	if (XMLStream::LoadXML(filePath.c_wstr(), materialDoc))
	{
		std::unique_ptr<Material> mat (new Material) ;
		auto root = materialDoc.child(TEXT("root"));
		if (!StringToGUID(guidStr.c_str(), &guid))
		{
			guid = CreateGUID();
			guidStr = GUIDToString(guid);
		}
		mat->_guid = guid;
		auto materialPrim = root.child(TEXT("MaterialPrimitive"));
		//MaterialPrimitive
		mat->_primitive.reset(new MaterialPrimitive());
		mat->_primitive->graphicsName = it->second->name;
		XMLStream::LoadXMLAttributeString(materialPrim, TEXT("vsShader"), mat->_primitive->vsShader);
		XMLStream::LoadXMLAttributeString(materialPrim, TEXT("psShader"), mat->_primitive->psShader);
		uint32_t pass;
		XMLStream::LoadXMLAttributeUInt(materialPrim, TEXT("pass"), pass);
		mat->_primitive->passUsing = (Pass)pass;
		auto vsCache = Shader::_vsShader[mat->_primitive->vsShader];
		auto psCache = Shader::_psShader[mat->_primitive->psShader];
		mat->_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(vsCache.header.vertexInput);
		//Parameters
		auto parameters = root.child(TEXT("Parameters"));
		int paramCount = 0;
		for (auto i = parameters.first_child(); i != NULL; i = i.next_sibling())
		{
			paramCount++;
		}
		mat->_paramterInfos.reserve(paramCount);
		mat->_primitive->uniformBuffer.reserve( paramCount);
		int alignmentFloat4 = 0; // float4 对齐
		glm::vec4 param = glm::vec4(0);
		uint32_t beginPos = 0;
		paramCount = 0;
		for (auto i = parameters.first_child(); i != NULL; i = i.next_sibling())
		{
			MaterialParameterInfo info = {};
			info.beginPos = beginPos;
			HString value;
			int type;
			XMLStream::LoadXMLAttributeString(i, TEXT("name"), info.name);
			//保证Shader的材质参数 和 材质文件的参数信息一致
			HString shaderParamName = psCache.params[paramCount].name;
			if (shaderParamName != info.name)
			{
				continue;
			}
			XMLStream::LoadXMLAttributeInt(i, TEXT("type"), type);
			info.type = (MPType)type;
			XMLStream::LoadXMLAttributeString(i, TEXT("value"), value);
			XMLStream::LoadXMLAttributeString(i, TEXT("ui"), info.ui);
			//
			auto splitValue = value.Split(",");
			if (alignmentFloat4 + splitValue.size() > 4)
			{
				mat->_primitive->uniformBuffer.push_back(param);
				alignmentFloat4 = 0;
				param = glm::vec4(0);
			}
			for (int i = 0; i < splitValue.size(); i++)
			{
				beginPos ++ ;
				param[alignmentFloat4] = (float)HString::ToDouble(splitValue[i]);
				alignmentFloat4++;
				if (alignmentFloat4 >= 4)
				{
					mat->_primitive->uniformBuffer.push_back(param);
					alignmentFloat4 = 0;
					param = glm::vec4(0);
				}
			}
			mat->_paramterInfos.push_back(info);
			paramCount++;
		}
		auto anlignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(glm::vec4) * mat->_primitive->uniformBuffer.size());
		mat->_primitive->uniformBufferSize = anlignmentSize;
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
	FileSystem::FileCopy((FileSystem::GetContentAbsPath() + TEXT("Core/Material/61A147FF-32BD-48EC-B523-57BC75EB16BA.mat")).c_str() ,newMatFilePath.c_str());


	return NULL;
}