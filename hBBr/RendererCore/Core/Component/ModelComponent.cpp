#include "ModelComponent.h"
#include "Resource/ModelData.h"
#include "Primitive.h"
#include "GameObject.h"
#include "FileSystem.h"
#include "ContentManager.h"
ModelComponent::ModelComponent(GameObject* parent) :Component(parent)
{
}

void ModelComponent::SetModelByRealPath(HString path)
{
	HGUID guid;
	path.CorrectionPath();
	HString guidStr = path.GetBaseName();
	StringToGUID(guidStr.c_str(), &guid);
	if (!_modelData.expired() && _modelData.lock()->guid == guid)
	{
		return;
	}
	SetModel(guid);
}

void ModelComponent::SetModelByVirtualPath(HString path)
{
	path.CorrectionPath();
	auto info = ContentManager::Get()->GetAssetInfo(AssetType::Model, path);
	if (info == NULL)
		return;
	if (!_modelData.expired() && _modelData.lock()->guid == info->guid)
	{
		return;
	}
	SetModel(info->guid);
}

void ModelComponent::SetModel(HGUID guid)
{
	if (!guid.isValid())
		return;
	ClearPrimitves();
	//create
	_modelData = ModelFileStream::ImportFbxToMemory(guid);
	if (!_modelData.expired())
	{
		ModelFileStream::BuildModelPrimitives(_modelData.lock().get(), _primitives);
		_materials.resize(_primitives.size());
		for (int i = 0; i < (int)_primitives.size(); i++)
		{
			_primitives[i]->transform = GetGameObject()->GetTransform();
			if (_materials[i].expired())
				_materials[i] = Material::LoadMaterial(HGUID("61A147FF-32BD-48EC-B523-57BC75EB16BA"));
			PrimitiveProxy::AddModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i]);
		}
	}
}

void ModelComponent::GameObjectActiveChanged(bool objActive)
{
	if (!_modelData.expired() && _bActive && objActive)
	{
		SetModel(_modelData.lock()->guid);
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
		if (_primitives.size() > i && _primitives[i] != NULL && !_materials[i].expired())
		{
			PrimitiveProxy::RemoveModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i]);
			delete _primitives[i];
			_primitives[i] = NULL;
		}
	}
	_primitives.clear();
}
