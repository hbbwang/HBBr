#include "Material.h"
#include "Shader.h"
#include "VertexFactory.h"

Material* Material::_defaultMaterial;

std::unordered_map<HUUID, std::shared_ptr<Material>> Material::_allMaterials;

Material::Material()
{
	_primitive.reset(new MaterialPrimitive());
	_primitive->graphicsName = _materialName;
	_primitive->vsShader = "BasePassTemplate";
	_primitive->psShader = "BasePassTemplate";
	_primitive->pass = _pass;
	_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(Shader::_vsShader[_primitive->vsShader].header.vertexInput);
	PrimitiveProxy::GetNewMaterialPrimitiveIndex(_primitive.get());
	PrimitiveProxy::AddMaterialPrimitive(_pass, _primitive.get());

}

Material::~Material()
{
	PrimitiveProxy::RemoveMaterialPrimitive(_pass, _primitive.get());
}

HUUID Material::LoadMaterial(const char* materialFilePath)
{
	//_allMaterials.emplace(std::make_pair(defaultMatGUID, newMat));
	return HUUID();
}

