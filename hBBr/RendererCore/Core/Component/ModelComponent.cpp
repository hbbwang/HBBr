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
	path.CorrectionPath();
	HString guidStr = path.GetBaseName();
	StringToGUID(guidStr.c_str(), &guid);
	if (_model == guid)
	{
		return;
	}
	_model = guid;
	SetModel(_model);
}

void ModelComponent::SetModel(HGUID guid)
{
	if (!_model.isValid())
		return;
	ClearPrimitves();
	//create
	_modelData = ModelFileStream::ImportFbxToMemory(guid);
	if (_modelData)
	{
		ModelFileStream::BuildModelPrimitives(_modelData, _primitives);
		_materials.resize(_primitives.size());
		for (int i = 0; i < (int)_primitives.size(); i++)
		{
			_primitives[i]->transform = GetGameObject()->GetTransform();
			if (_materials[i] == NULL)
				_materials[i] = Material::LoadMaterial(HGUID("61A147FF-32BD-48EC-B523-57BC75EB16BA"));
			PrimitiveProxy::AddModelPrimitive(_materials[i]->GetPrimitive(), _primitives[i]);
		}
	}
}

void ModelComponent::GameObjectActiveChanged(bool objActive)
{
	if (_bActive && objActive)
	{
		SetModel(_model);
	}
	else
	{
		ClearPrimitves();
	}
}

void ModelComponent::Update()
{
}

void ModelComponent::ExecuteDestroy()
{
	ClearPrimitves();
}

void ModelComponent::ClearPrimitves()
{
	//clear
	if (_primitives.size() <= 0)
		return;
	for (int i = 0; i < (int)_materials.size(); i++)
	{
		if (_primitives.size() > i && _primitives[i] != NULL)
		{
			PrimitiveProxy::RemoveModelPrimitive(_materials[i]->GetPrimitive(), _primitives[i]);
			delete _primitives[i];
			_primitives[i] = NULL;
		}
	}
	_primitives.clear();
}
