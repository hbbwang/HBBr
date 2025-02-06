#include "ModelComponent.h"
#include "Asset/Model.h"
#include "Primitive.h"
#include "GameObject.h"
#include "FileSystem.h"
#include "ContentManager.h"
#include "World.h"
// 
COMPONENT_IMPLEMENT(ModelComponent)

void ModelComponent::OnConstruction()
{
	Component::OnConstruction();
	AddProperty("AssetRef", "Model", &_model, false, "Default", 0, "fbx", false);
	AddProperty("AssetRef", "Materials", &_materials, true, "Default", 0, "mat", false);
}

void ModelComponent::UpdateMaterial()
{
	_materials.resize(_primitives.size());
	for (int i = 0; i < (int)_primitives.size(); i++)
	{
		_primitives[i]->transform = GetGameObject()->GetTransform();
		if (!_materials[i].asset)
		{
			auto defaultMaterial = Material::GetDefaultMaterial();
			_materials[i].asset = defaultMaterial.lock();
		}
		auto matAsset = AssetObject::Cast<Material>(_materials[i].asset);
		PrimitiveProxy::AddModelPrimitive(matAsset->GetPrimitive(), _primitives[i], _gameObject->GetWorld()->GetRenderer());

		_materials[i].path = _materials[i].asset->_assetInfo.lock()->virtualFilePath;
		_materials[i].callBack();
	}
}

void ModelComponent::SetModelByAssetPath(std::string virtualPath)
{
	if (!_bActive || !_gameObject->IsActive() || !_gameObject->GetLevel()->IsActive())
		return;
	auto info = ContentManager::Get()->GetAssetInfo(virtualPath);
	if (info.expired())
		return;
	auto model = info.lock()->GetAssetObject<Model>();
	if (model)
	{
		SetModel(model);
	}
}

void ModelComponent::SetModel(HGUID guid)
{
	if (!guid.isValid() || !_bActive || !_gameObject->IsActive() || !_gameObject->GetLevel()->IsActive())
		return;
	//create
	SetModel(Model::LoadAsset(guid));
}

void ModelComponent::SetModel(std::shared_ptr<class Model> model, std::vector<std::shared_ptr<class Material>> *mats)
{
	if (!_bActive || !_gameObject->IsActive() || !_gameObject->GetLevel()->IsActive())
		return;
	if (model)
	{
		_model.asset = model;
		_model.path = model->_assetInfo.lock()->virtualFilePath;
		_model.callBack();
		ClearPrimitves();
		Model::BuildModelPrimitives(model.get(), _primitives, this->_renderer);
		if (mats!=nullptr && mats->size()>0)
		{
			const int inputMatCount = (int)mats->size();
			const int matCount = (int)_materials.size();
			if (inputMatCount > matCount)
			{
				mats->resize(_primitives.size());
			}
			for (int i = 0; i < matCount; i++)
			{
				if (i < inputMatCount)
				{
					_materials[i].asset = mats->at(i);
					_materials[i].path = mats->at(i)->_assetInfo.lock()->virtualFilePath;
					_materials[i].callBack();
				}
			}
		}
		UpdateMaterial();
	}
}

void ModelComponent::SetMaterial(std::shared_ptr<class Material> mat, int index)
{
	if (index >= _materials.size())
	{
		ConsoleDebug::printf_endl_warning(GetInternationalizationText("ModelComponent","SetMaterialIndexError"));
		return;
	}
	if (mat && _model.asset)
	{
		ClearPrimitves();
		Model::BuildModelPrimitives(AssetObject::Cast<Model>(_model.asset).get(), _primitives, this->_renderer);
		_materials[index].asset = mat;
		_materials[index].path = mat->_assetInfo.lock()->virtualFilePath;
		_materials[index].callBack();
		UpdateMaterial();
	}
}

void ModelComponent::GameObjectActiveChanged(bool objActive)
{
	if ( _bActive && objActive && _gameObject->GetLevel()->IsActive())
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
	//Model
	auto modelInfo = ContentManager::Get()->GetAssetByVirtualPath(_model.path);
	if (!modelInfo.expired())
	{
		_model.asset = modelInfo.lock()->GetSharedAssetObject();
	}

	if (_model.asset)
	{
		_bErrorStatus = false;
		//Material
		std::vector<std::shared_ptr<class Material>> mats;
		mats.resize(_materials.size());
		const int count = (int)_materials.size();
		for (int i = 0; i < count; i++)
		{
			auto info = ContentManager::Get()->GetAssetByVirtualPath(_materials[i].path);
			if (!info.expired())
			{
				mats[i] = info.lock()->GetAssetObject<Material>();
			}
			else
			{
				mats[i] = Material::GetErrorMaterial().lock();
			}
		}
		SetModel(AssetObject::Cast<Model>(_model.asset), &mats);
	}
	else
	{
		_errorModelPath = _model.path;
		_bErrorStatus = true;
		_model.asset = ContentManager::Get()->GetAssetInfo(HGUID("0a13c94b-ff3a-4786-bc1b-10e420a23bf4"), AssetType::Model).lock()->GetSharedAssetObject();
		SetModel(AssetObject::Cast<Model>(_model.asset));
		for (int i = 0; i < GetMaterialNum(); i++)
		{
			SetMaterial(Material::GetErrorMaterial().lock(), i);
		}
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
	for (int i = 0; i < (int)_materials.size(); i++)
	{
		if (_primitives.size() > i && _primitives[i] != nullptr && _materials[i].asset)
		{
			PrimitiveProxy::RemoveModelPrimitive(
				AssetObject::Cast<Material>(_materials[i].asset)->GetPrimitive(),
				_primitives[i], 
				_gameObject->GetWorld()->GetRenderer());
			delete _primitives[i];
			_primitives[i] = nullptr;
		}
	}
	_primitives.clear();
}

void ModelComponent::Deserialization(nlohmann::json input)
{
	nlohmann::json compPros = input["Properties"];
	for (auto& p : compPros.items())
	{
		std::string proName = p.key();
		std::string proType = p.value()["Type"];
		std::string proValue = p.value()["Value"];
		for (auto& pp : _compProperties)
		{
			if (pp.name == proName && pp.type == proType)
			{
				Component::StringToPropertyValue(pp, proValue);
				break;
			}
		}
	}
}

nlohmann::json ModelComponent::Serialization()
{
	nlohmann::json sub;
	nlohmann::json subPros;
	sub["Class"] = GetComponentName();
	//Variables
	for (auto& p : _compProperties)
	{
		nlohmann::json subPro;
		auto valueStr = Component::PropertyValueToString(p);
		if (valueStr.length() > 0)
		{
			subPro["Type"] = p.type;
			subPro["Value"] = valueStr;
			//错误状态,还是继续保存错误数据进去
			if (p.name == "Model")
			{
				if(_bErrorStatus)
					subPro["Value"] = _errorModelPath;
			}
			subPros[p.name.c_str()] = subPro;
		}
	}
	sub["Properties"] = subPros;
	return sub;
}