#pragma once
#include "Primitive.h"
#include "Shader.h"

std::vector<std::vector<MaterialPrimitive*>> PrimitiveProxy::_allGraphicsPrimitives;

std::map<MaterialPrimitive* ,std::vector<ModelPrimitive>> PrimitiveProxy::_allModelPrimitives;

void PrimitiveProxy::AddMaterialPrimitive(Pass pass, MaterialPrimitive* prim)
{
	if ((uint32_t)PrimitiveProxy::_allGraphicsPrimitives.size() != (uint32_t)Pass::MaxNum)
	{
		_allGraphicsPrimitives.resize((uint32_t)Pass::MaxNum);
	}
	_allGraphicsPrimitives[(uint32_t)pass].push_back(prim);
}

void PrimitiveProxy::GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim)
{
	uint64_t result = 0;
	prim->graphicsIndex.vsIndex = Shader::_vsShader[prim->vsShader].shaderCacheIndex;
	prim->graphicsIndex.psIndex = Shader::_vsShader[prim->vsShader].shaderCacheIndex;
	prim->graphicsIndex.varients = prim->varients;
	//未来还会有混合模式的识别一起加入进来，目前就这样
}

void PrimitiveProxy::RemoveMaterialPrimitive(Pass pass, MaterialPrimitive* prim)
{
	//auto mit = _allModelPrimitives.find(prim);
	//if (mit != _allModelPrimitives.end())
	//{
	//	_allModelPrimitives.erase(mit);
	//}

	if ((uint32_t)PrimitiveProxy::_allGraphicsPrimitives.size() != (uint32_t)Pass::MaxNum)
	{
		_allGraphicsPrimitives.resize((uint32_t)Pass::MaxNum);
	}
	int index = (uint32_t)pass;
	auto it = std::find(_allGraphicsPrimitives[index].begin(), _allGraphicsPrimitives[index].end(), prim);
	if (it != _allGraphicsPrimitives[index].end())
	{
		_allGraphicsPrimitives[index].erase(it);
	}
}

void PrimitiveProxy::AddModelPrimitive(MaterialPrimitive* mat, ModelPrimitive prim)
{
	prim.vertexData =  prim.vertexInput.GetData(Shader::_vsShader[mat->vsShader].header.vertexInput);
	prim.vertexInput = VertexFactory::VertexInput();
	_allModelPrimitives[mat].push_back(prim);
}

void PrimitiveProxy::RemoveModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim)
{
	auto it = _allModelPrimitives.find(mat);
	if (it != _allModelPrimitives.end())
	{
		auto pit = std::find_if(it->second.begin(), it->second.end(), [prim](ModelPrimitive& model)
			{
				return model.modelPrimitiveName == prim->modelPrimitiveName;
			});
		if (pit != it->second.end())
		{
			it->second.erase(pit);
		}
	}
}
