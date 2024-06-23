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
	AddProperty("AssetRef", "Materials", &_materialPath, true, "Default", 0, "mat");
}

void ModelComponent::UpdateMaterial()
{
	_materialPath.resize(_primitives.size());
	for (int i = 0; i < (int)_primitives.size(); i++)
	{
		_primitives[i]->transform = GetGameObject()->GetTransform();
		if (_materialPath[i].assetInfo.expired())
		{
			auto defaultMaterial = Material::GetDefaultMaterial();
			_materialPath[i].assetInfo = defaultMaterial.lock()->_assetInfo;
		}
		auto matAsset = _materialPath[i].assetInfo.lock()->GetAssetObject<Material>();
		PrimitiveProxy::AddModelPrimitive(matAsset.lock()->GetPrimitive(), _primitives[i], _gameObject->GetWorld()->GetRenderer());

		_materialPath[i].path = _materialPath[i].assetInfo.lock()->virtualFilePath;
		_materialPath[i].callBack();
	}
}

void ModelComponent::SetModelByAssetPath(HString virtualPath)
{
	if (!_bActive || !_gameObject->IsActive())
		return;
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
	if (!guid.isValid() || !_bActive || !_gameObject->IsActive() )
		return;
	//create
	SetModel(Model::LoadAsset(guid));
}

void ModelComponent::SetModel(std::weak_ptr<class Model> model)
{
	if (!_bActive || !_gameObject->IsActive())
		return;
	ClearPrimitves();
	if (!model.expired())
	{
		Model::BuildModelPrimitives(model.lock().get(), _primitives);
		UpdateMaterial();
		_modelPath.assetInfo = model.lock()->_assetInfo;
		_modelPath.path = model.lock()->_assetInfo.lock()->virtualFilePath;
		_modelPath.callBack();
	}
}

void ModelComponent::SetMaterial(std::weak_ptr<class Material> mat, int index)
{
	if (index >= _materialPath.size())
	{
		ConsoleDebug::printf_endl_warning(GetInternationalizationText("ModelComponent","SetMaterialIndexError"));
		return;
	}
	if (!mat.expired())
	{
		_materialPath[index].assetInfo = mat.lock()->_assetInfo;
		_materialPath[index].path = mat.lock()->_assetInfo.lock()->virtualFilePath;
		_materialPath[index].callBack();
		UpdateData();
	}
}

void ModelComponent::GameObjectActiveChanged(bool objActive)
{
	if ( _bActive && objActive )
	{
		UpdateData();
	}
	else
	{
		ClearPrimitves();
	}
}

void ModelComponent::UpdateData()
{
	_modelPath.assetInfo = ContentManager::Get()->GetAssetByVirtualPath(_modelPath.path);
	for (int i = 0; i < (int)_materialPath.size(); i++)
	{
		auto info = ContentManager::Get()->GetAssetByVirtualPath(_materialPath[i].path);
		if(!info.expired())
			_materialPath[i].assetInfo = info;
	}
	if (!_modelPath.assetInfo.expired())
	{
		SetModel(_modelPath.assetInfo.lock()->GetAssetObject<Model>());
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
	for (int i = 0; i < (int)_materialPath.size(); i++)
	{
		if (_primitives.size() > i && _primitives[i] != nullptr && !_materialPath[i].assetInfo.expired())
		{

			PrimitiveProxy::RemoveModelPrimitive(
				_materialPath[i].assetInfo.lock()->GetAssetObject<Material>().lock()->GetPrimitive(), 
				_primitives[i], 
				_gameObject->GetWorld()->GetRenderer());

			delete _primitives[i];
			_primitives[i] = nullptr;
		}
	}
	_primitives.clear();
}
