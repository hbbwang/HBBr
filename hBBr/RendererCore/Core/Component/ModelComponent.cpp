#include "ModelComponent.h"
#include "Resource/ModelData.h"
#include "Primitive.h"
#include "GameObject.h"
#include "FileSystem.h"
#include "ContentManager.h"
#include "World.h"
COMPONENT_IMPLEMENT(ModelComponent)

void ModelComponent::OnConstruction()
{
	Component::OnConstruction();
	AddProperty("Model",  &_modelData , "");
	AddProperty("Material", &_materials , "");
}

void ModelComponent::SetModelByRealPath(HString path)
{
	HGUID guid;
	FileSystem::CorrectionPath(path);
	HString guidStr = path.GetBaseName();
	StringToGUID(guidStr.c_str(), &guid);
	if (!_modelData.expired() && _modelData.lock()->_assetInfo->guid == guid)
	{
		return;
	}
	SetModel(guid);
}

void ModelComponent::SetModelByVirtualPath(HString path)
{
	FileSystem::CorrectionPath(path);
	auto info = ContentManager::Get()->GetAssetInfo(AssetType::Model, path);
	if (info == NULL)
		return;
	if (!_modelData.expired() && _modelData.lock()->_assetInfo->guid == info->guid)
	{
		return;
	}
	SetModel(info->guid);
}

void ModelComponent::SetModel(HGUID guid)
{
	if (!guid.isValid())
		return;
	//create
	_modelData = ModelData::LoadAsset(guid);
	SetModel(_modelData);
}

void ModelComponent::SetModel(std::weak_ptr<class ModelData> model)
{
	ClearPrimitves();
	if (!model.expired())
	{
		_lastModelData = model;
		ModelData::BuildModelPrimitives(model.lock().get(), _primitives);
		_materials.resize(_primitives.size());
		for (int i = 0; i < (int)_primitives.size(); i++)
		{
			_primitives[i]->transform = GetGameObject()->GetTransform();
			if (_materials[i].expired())
				_materials[i] = Material::LoadAsset(HGUID("61A147FF-32BD-48EC-B523-57BC75EB16BA"));
			PrimitiveProxy::AddModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i], _gameObject->GetScene()->GetRenderer());
		}
	}
}

void ModelComponent::GameObjectActiveChanged(bool objActive)
{
	if (!_modelData.expired() && _bActive && objActive)
	{
		SetModel(_modelData.lock()->_assetInfo->guid);
	}
	else
	{
		ClearPrimitves();
	}
}

void ModelComponent::Update()
{
	if (_modelData.lock().get() != _lastModelData.lock().get())
	{
		SetModel(_modelData);
	}
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
			PrimitiveProxy::RemoveModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i], _gameObject->GetScene()->GetRenderer());
			delete _primitives[i];
			_primitives[i] = NULL;
		}
	}
	_primitives.clear();
}
