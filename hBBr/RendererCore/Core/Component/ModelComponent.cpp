#include "ModelComponent.h"
#include "Asset/ModelData.h"
#include "Primitive.h"
#include "GameObject.h"
#include "FileSystem.h"
#include "ContentManager.h"
#include "World.h"
COMPONENT_IMPLEMENT(ModelComponent)

void ModelComponent::OnConstruction()
{
	Component::OnConstruction();
	AddProperty(ModelData, "Model", &_modelGUID, false, "Default", 0);
	AddProperty(Material, "Material", &_materialGUIDs, false, "Default", 0);
}

void ModelComponent::SetModelByRealPath(HString path)
{
	HGUID guid;
	FileSystem::CorrectionPath(path);
	HString guidStr = path.GetBaseName();
	StringToGUID(guidStr.c_str(), &guid);
	if (!_modelData.expired()  && _modelData.lock()->_assetInfo->guid == guid)
	{
		return;
	}
	SetModel(guid);
}

void ModelComponent::SetModelByVirtualPath(HString path)
{
	path = FileSystem::FillUpAssetPath(path);
	auto info = ContentManager::Get()->GetAssetInfo(AssetType::Model, path);
	if (info.expired())
		return;
	if (!_modelData.expired() && _modelData.lock()->_assetInfo->guid == info.lock()->guid)
	{
		return;
	}
	SetModel(info.lock()->guid);
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
		_modelGUID = model.lock()->_assetInfo->guid;
		_oldModelGUID = model.lock()->_assetInfo->guid;
		ModelData::BuildModelPrimitives(model.lock().get(), _primitives);
		_materials.resize(_primitives.size());
		for (int i = 0; i < (int)_primitives.size(); i++)
		{
			_primitives[i]->transform = GetGameObject()->GetTransform();
			if (_materials[i].expired())
				_materials[i] = Material::LoadAsset(HGUID("61A147FF-32BD-48EC-B523-57BC75EB16BA"));
			PrimitiveProxy::AddModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i], _gameObject->GetWorld()->GetRenderer());
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
	if (_modelGUID != _oldModelGUID)
	{
		SetModel(_modelGUID);
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
		if (_primitives.size() > i && _primitives[i] != nullptr && !_materials[i].expired())
		{
			PrimitiveProxy::RemoveModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i], _gameObject->GetWorld()->GetRenderer());
			delete _primitives[i];
			_primitives[i] = nullptr;
		}
	}
	_primitives.clear();
}
