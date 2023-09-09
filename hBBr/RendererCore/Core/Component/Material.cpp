#include "Material.h"
#include "Shader.h"
#include "VertexFactory.h"
#include "VulkanManager.h"
Material* Material::_defaultMaterial;

Material* Material::_errorMaterial;

std::unordered_map<HUUID, std::unique_ptr<Material>> Material::_allMaterials;

Material::Material(bool bDefault)
{
	if (bDefault)
	{
		_primitive.reset(new MaterialPrimitive());
		_primitive->graphicsName = _materialName;
		_primitive->vsShader = "BasePassTemplate";
		_primitive->psShader = "BasePassTemplate";
		_primitive->passUsing = Pass::OpaquePass;
		_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(Shader::_vsShader[_primitive->vsShader].header.vertexInput);
		PrimitiveProxy::GetNewMaterialPrimitiveIndex(_primitive.get());
		PrimitiveProxy::AddMaterialPrimitive(_primitive.get());
	}
}

Material::~Material()
{
	PrimitiveProxy::RemoveMaterialPrimitive(_primitive->passUsing, _primitive.get());
}

Material* Material::LoadMaterial(HString materialFilePath)
{
	pugi::xml_document materialDoc;
	if (XMLStream::LoadXML(materialFilePath.c_wstr(), materialDoc))
	{
		std::unique_ptr<Material> mat (new Material) ;
		auto root = materialDoc.child(TEXT("root"));
		HUUID uuid;
		HString uuidStr;
		XMLStream::LoadXMLAttributeString(root, TEXT("UUID"), uuidStr);
		if (!StringToUUID(uuidStr.c_str(), &uuid))
		{
			uuid = CreateUUID();
			uuidStr = UUIDToString(uuid).c_str();
		}
		auto materialPrim = root.child(TEXT("MaterialPrimitive"));
		//MaterialPrimitive
		mat->_primitive.reset(new MaterialPrimitive());
		mat->_primitive->graphicsName = materialFilePath.GetBaseName();
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
			MaterialParameterInfo info;
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
		_allMaterials.emplace(std::make_pair(uuid, std::move(mat)));
		return _allMaterials[uuid].get();
	}
	return NULL;
}

