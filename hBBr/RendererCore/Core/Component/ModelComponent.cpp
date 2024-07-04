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
	AddProperty("AssetRef", "Model", &_model, false, "Default", 0, "fbx", false);
	AddProperty("AssetRef", "Materials", &_materials, true, "Default", 0, "mat", false);
}

void ModelComponent::UpdateMaterial()
{
	_materials.resize(_primitives.size());
	for (int i = 0; i < (int)_primitives.size(); i++)
	{
		_primitives[i]->transform = GetGameObject()->GetTransform();
		if (_materials[i].assetInfo.expired())
		{
			auto defaultMaterial = Material::GetDefaultMaterial();
			_materials[i].assetInfo = defaultMaterial.lock()->_assetInfo;
		}
		auto matAsset = _materials[i].assetInfo.lock()->GetAssetObject<Material>();
		PrimitiveProxy::AddModelPrimitive(matAsset.lock()->GetPrimitive(), _primitives[i], _gameObject->GetWorld()->GetRenderer());

		_materials[i].path = _materials[i].assetInfo.lock()->virtualFilePath;
		_materials[i].callBack();
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

void ModelComponent::SetModel(std::weak_ptr<class Model> model, std::vector<std::weak_ptr<class Material>> *mats)
{
	if (!_bActive || !_gameObject->IsActive())
		return;
	if (!model.expired())
	{
		_model.assetInfo = model.lock()->_assetInfo;
		_model.path = model.lock()->_assetInfo.lock()->virtualFilePath;
		_model.callBack();
		ClearPrimitves();
		Model::BuildModelPrimitives(_model.assetInfo.lock()->GetAssetObject<Model>().lock().get(), _primitives, this->_renderer);
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
					_materials[i].assetInfo = mats->at(i).lock()->_assetInfo;
					_materials[i].path = mats->at(i).lock()->_assetInfo.lock()->virtualFilePath;
					_materials[i].callBack();
				}
			}
		}
		UpdateMaterial();
	}
}

void ModelComponent::SetMaterial(std::weak_ptr<class Material> mat, int index)
{
	if (index >= _materials.size())
	{
		ConsoleDebug::printf_endl_warning(GetInternationalizationText("ModelComponent","SetMaterialIndexError"));
		return;
	}
	if (!mat.expired() && !_model.assetInfo.expired())
	{
		ClearPrimitves();
		Model::BuildModelPrimitives(_model.assetInfo.lock()->GetAssetObject<Model>().lock().get(), _primitives, this->_renderer);
		_materials[index].assetInfo = mat.lock()->_assetInfo;
		_materials[index].path = mat.lock()->_assetInfo.lock()->virtualFilePath;
		_materials[index].callBack();
		UpdateMaterial();
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
	//Model
	_model.assetInfo = ContentManager::Get()->GetAssetByVirtualPath(_model.path);
	if (!_model.assetInfo.expired())
	{
		_bErrorStatus = false;
		//Material
		std::vector<std::weak_ptr<class Material>> mats;
		mats.resize(_materials.size());
		const int count = (int)_materials.size();
		for (int i = 0; i < count; i++)
		{
			auto info = ContentManager::Get()->GetAssetByVirtualPath(_materials[i].path);
			if (!info.expired())
			{
				mats[i] = info.lock()->GetAssetObject<Material>();
			}
		}
		SetModel(_model.assetInfo.lock()->GetAssetObject<Model>(), &mats);
	}
	else
	{
		_errorModelPath = _model.path;
		_bErrorStatus = true;
		_model.assetInfo = ContentManager::Get()->GetAssetInfo(HGUID("0a13c94b-ff3a-4786-bc1b-10e420a23bf4"), AssetType::Model);
		SetModel(_model.assetInfo.lock()->GetAssetObject<Model>());
		for (int i = 0; i < GetMaterialNum(); i++)
		{
			SetMaterial(Material::GetErrorMaterial(), i);
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
		if (_primitives.size() > i && _primitives[i] != nullptr && !_materials[i].assetInfo.expired())
		{
			PrimitiveProxy::RemoveModelPrimitive(
				_materials[i].assetInfo.lock()->GetAssetObject<Material>().lock()->GetPrimitive(), 
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
		HString proName = p.key();
		HString proType = p.value()["Type"];
		HString proValue = p.value()["Value"];
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
		if (valueStr.Length() > 0)
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