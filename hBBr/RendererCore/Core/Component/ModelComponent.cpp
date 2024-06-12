#include "ModelComponent.h"
#include "Asset/Model.h"
#include "Primitive.h"
#include "GameObject.h"
#include "FileSystem.h"
#include "ContentManager.h"
#include "World.h"
COMPONENT_IMPLEMENT(ModelComponent)

void ModelComponent::OnConstruction()
{
	Component::OnConstruction();
	AddProperty("AssetRef", "Model", &_modelPath, false, "Default", 0, "fbx");
	AddProperty("AssetRef", "Material", &_materialPath, true, "Default", 0, "mat");
}

void ModelComponent::SetModelByAssetPath(HString virtualPath)
{
	auto info = ContentManager::Get()->GetAssetInfo(virtualPath);
	if (info.expired())
		return;
	auto model = info.lock()->GetAssetObject<Model>();
	if (!model.expired())
	{
		SetModel(model);
	}
}

void ModelComponent::SetModel(HGUID guid)
{
	if (!guid.isValid())
		return;
	//create
	SetModel(Model::LoadAsset(guid));
}

void ModelComponent::SetModel(std::weak_ptr<class Model> model)
{
	ClearPrimitves();
	if (!model.expired())
	{
		Model::BuildModelPrimitives(model.lock().get(), _primitives);
		_materials.resize(_primitives.size());
		for (int i = 0; i < (int)_primitives.size(); i++)
		{
			_primitives[i]->transform = GetGameObject()->GetTransform();
			if (_materials[i].expired())
				_materials[i] = Material::LoadAsset(HGUID("b51e2e9a-0985-75e8-6138-fa95efcbab57"));
			PrimitiveProxy::AddModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i], _gameObject->GetWorld()->GetRenderer());
		}
		_modelPath.path = model.lock()->_assetInfo->virtualFilePath;
		_modelPath.callBack();
	}
}

void ModelComponent::GameObjectActiveChanged(bool objActive)
{
	if ( _bActive && objActive )
	{
		auto info = ContentManager::Get()->GetAssetByVirtualPath(_modelPath.path);
		if (!info.expired())
		{
			SetModel(info.lock()->GetAssetObject<Model>());
		}
	}
	else
	{
		ClearPrimitves();
	}
}

void ModelComponent::UpdateData()
{
	auto info = ContentManager::Get()->GetAssetByVirtualPath(_modelPath.path);
	if (!info.expired())
	{
		SetModel(info.lock()->GetAssetObject<Model>());
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
		if (_primitives.size() > i && _primitives[i] != nullptr && !_materials[i].expired())
		{
			PrimitiveProxy::RemoveModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i], _gameObject->GetWorld()->GetRenderer());
			delete _primitives[i];
			_primitives[i] = nullptr;
		}
	}
	_primitives.clear();
}
