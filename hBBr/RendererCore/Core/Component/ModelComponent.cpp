#include"ModelComponent.h"
#include "Resource/ModelData.h"
#include "Primitive.h"
#include "GameObject.h"

ModelComponent::ModelComponent(GameObject* parent) :Component(parent)
{

}

void ModelComponent::SetModel(HString path)
{
	path.CorrectionPath();
	_modelData = ModelFileStream::ImportFbxToMemory(path);
	std::vector<GraphicsPrimitive> prims;
	ModelFileStream::BuildGraphicsPrimitives(_modelData, prims);
	_pass.resize(prims.size());
	_vsShader = "BasePassVertexShader";
	_psShader = "BasePassPixelShader";
	for (int i = 0; i < prims.size(); i++)
	{
		PrimitiveProxy::AddGraphicsPrimitives(_pass[i], _vsShader, _psShader, prims[i]);
	}
}
