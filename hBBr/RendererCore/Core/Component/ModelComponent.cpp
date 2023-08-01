#include"ModelComponent.h"
#include "Resource/ModelData.h"
#include "Primitive.h"

ModelComponent::ModelComponent(GameObject* parent) :Component(parent)
{

}

void ModelComponent::SetModel(HString path)
{
	path.CorrectionPath();
	_modelData = ModelFileStream::ImportFbxToMemory(path);
	ModelPrimitive prim = {};
	ModelFileStream::BuildModelPrimitive(_modelData, prim);
	prim.bBasePass = true;
	PrimitiveProxy::AddModelPrimitive(Pass::BasePass, prim);
}
