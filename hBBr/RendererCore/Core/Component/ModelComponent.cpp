#include "ModelComponent.h"
#include "Resource/ModelData.h"
#include "Primitive.h"
#include "GameObject.h"
#include "FileSystem.h"
ModelComponent::ModelComponent(GameObject* parent) :Component(parent)
{
}

void ModelComponent::SetModel(HString path)
{
	//clear
	for (int i = 0; i < (int)_materials.size(); i++)
	{
		PrimitiveProxy::RemoveModelPrimitive(_materials[i]->GetPrimitive(), &_primitive[i]);
	}
	_primitive.clear();
	//create
	path.CorrectionPath();
	_modelData = ModelFileStream::ImportFbxToMemory(path);
	ModelFileStream::BuildModelPrimitives(_modelData, _primitive);
	_materials.resize(_primitive.size());
	for (int i = 0; i < (int)_primitive.size(); i++)
	{
		_primitive[i].transform = GetGameObject()->GetTransform();
		if (_materials[i] == NULL)
			//_materials[i] = Material::GetDefaultMaterial();//Set default material
			_materials[i] = Material::LoadMaterial(FileSystem::GetContentAbsPath() + "Core/Material/DefaultPBR.mat");
		PrimitiveProxy::AddModelPrimitive(_materials[i]->GetPrimitive(), &_primitive[i]);
	}
}

void ModelComponent::SetActive(bool newActive)
{
	Component::SetActive(newActive);
	if (!newActive)
	{

	}
}

void ModelComponent::ExecuteDestroy()
{
	for (int i = 0; i < (int)_materials.size(); i++)
	{
		PrimitiveProxy::RemoveModelPrimitive(_materials[i]->GetPrimitive(), &_primitive[i]);
	}
	_primitive.clear();
}
