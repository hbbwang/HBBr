#include "ModelComponent.h"
#include "Resource/ModelData.h"
#include "Primitive.h"
#include "GameObject.h"
#include "FileSystem.h"
ModelComponent::ModelComponent(GameObject* parent) :Component(parent)
{
	_vsShader = "BasePassVertexShader";
	_psShader = "BasePassPixelShader";
}

void ModelComponent::SetModel(HString path)
{
	//先清除
	if (_modelPrimitiveID.Length() > 5)
	{
		for (int i = 0; i < _pass.size(); i++)
		{
			PrimitiveProxy::RemoveModelPrimitives(_pass[i], _graphicsPrimID[i], _modelPrimitiveID);
		}
	}
	//再获取
	path.CorrectionPath();
	_modelData = ModelFileStream::ImportFbxToMemory(path);
	if (_modelData != NULL)
	{
		_modelPrimitiveID = FileSystem::GetRelativePath(path.c_str());
		_pass.clear();
		_graphicsPrimID.clear();
		std::vector<GraphicsPrimitive> prims;
		ModelFileStream::BuildGraphicsPrimitives(_modelData, prims);
		_pass.resize(prims.size());
		_graphicsPrimID.resize(prims.size());
		for (int i = 0; i < prims.size(); i++)
		{
			for (auto& p : prims[i].modelPrimitives)
				p.transform = GetGameObject()->GetTransform();
			_graphicsPrimID[i] = PrimitiveProxy::AddGraphicsPrimitives(_pass[i], _vsShader, _psShader, prims[i]);
		}
	}
}

void ModelComponent::SetActive(bool newActive)
{
	Component::SetActive(newActive);
	if (!newActive)
	{

	}
}
