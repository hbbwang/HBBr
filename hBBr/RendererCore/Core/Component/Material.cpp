#include "Material.h"
#include "Shader.h"
#include "VertexFactory.h"

std::shared_ptr<Material> Material::_defaultMaterial;

Material::Material()
{
	_primitive.reset(new MaterialPrimitive());
	_primitive->graphicsName = _materialName;
	_primitive->vsShader = "BasePassVertexShader";
	_primitive->psShader = "BasePassPixelShader";
	_primitive->inputLayout = VertexFactory::VertexInput::BuildLayout(Shader::_vsShader[_primitive->vsShader].header.vertexInput);
	PrimitiveProxy::GetNewMaterialPrimitiveIndex(_primitive.get());
	PrimitiveProxy::AddMaterialPrimitive(_pass, _primitive.get());
}

Material::~Material()
{
	PrimitiveProxy::RemoveMaterialPrimitive(_pass, _primitive.get());
}

