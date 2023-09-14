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
	HGUID guid;
	HString guidStr = path.GetBaseName();
	StringToGUID(guidStr.c_str(), &guid);
	if (_model == guid)
	{
		return;
	}
	_model = guid;
	//clear
	for (int i = 0; i < (int)_materials.size(); i++)
	{
		PrimitiveProxy::RemoveModelPrimitive(_materials[i]->GetPrimitive(), &_primitive[i]);
	}
	_primitive.clear();
	//create
	_modelData = ModelFileStream::ImportFbxToMemory(guid);
	ModelFileStream::BuildModelPrimitives(_modelData, _primitive);
	_materials.resize(_primitive.size());
	for (int i = 0; i < (int)_primitive.size(); i++)
	{
		_primitive[i].transform = GetGameObject()->GetTransform();
		if (_materials[i] == NULL)
			//_materials[i] = Material::GetDefaultMaterial();//Set default material
			_materials[i] = Material::LoadMaterial(HGUID("61A147FF-32BD-48EC-B523-57BC75EB16BA"));
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
