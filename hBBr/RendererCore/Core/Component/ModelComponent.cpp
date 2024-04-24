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
	AddProperty(Model, "Model", &_modelGUID, false, "Default", 0);
	AddProperty(Material, "Material", &_materialGUIDs, false, "Default", 0);
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
		_modelCache = model;
		_modelGUID = model.lock()->_assetInfo->guid;
		_oldModelGUID = model.lock()->_assetInfo->guid;
		Model::BuildModelPrimitives(model.lock().get(), _primitives);
		_materialGUIDs.resize(_primitives.size());
		_materials.resize(_primitives.size());
		for (int i = 0; i < (int)_primitives.size(); i++)
		{
			_primitives[i]->transform = GetGameObject()->GetTransform();
			if (_materials[i].expired())
				_materials[i] = Material::LoadAsset(HGUID("b51e2e9a-0985-75e8-6138-fa95efcbab57"));
			PrimitiveProxy::AddModelPrimitive(_materials[i].lock()->GetPrimitive(), _primitives[i], _gameObject->GetWorld()->GetRenderer());
		}
	}
}

void ModelComponent::GameObjectActiveChanged(bool objActive)
{
	if ( _bActive && objActive )
	{
		SetModel(_modelGUID);
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
