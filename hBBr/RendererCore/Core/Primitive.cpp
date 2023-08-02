#pragma once
#include "Primitive.h"
#include "Shader.h"
std::map<Pass, std::vector<GraphicsPrimitive>> PrimitiveProxy::_allGraphicsPrimitives;

HString PrimitiveProxy::AddGraphicsPrimitives(Pass pass, HString vsShader, HString psShader, GraphicsPrimitive prim)
{
	//Generate ID
	prim.graphicsID = vsShader + psShader ;//Graphics唯一标识
	//Build layout and data
	uint8_t intputs[6]; 
	memcpy(intputs, Shader::_vsShader[vsShader].header.vertexInput, sizeof(uint8_t) * 6);
	prim.inputLayout = VertexFactory::VertexInput::BuildLayout(intputs);
	for (auto& i : prim.modelPrimitives)
	{
		i.vertexData = i.vertexInput.GetData(intputs);
		i.vertexInput = VertexFactory::VertexInput();
	}
	//
	auto it = _allGraphicsPrimitives.find(pass);
	if (it == _allGraphicsPrimitives.end())
	{
		prim.vsShader = vsShader;
		prim.psShader = psShader;
		_allGraphicsPrimitives.emplace(std::make_pair(pass, std::vector<GraphicsPrimitive>({ prim })));
	}
	else
	{
		auto pit = std::find_if(it->second.begin(), it->second.end(), [prim, vsShader, psShader](GraphicsPrimitive& gp)
			{
				return prim.graphicsID == gp.graphicsID;
			});
		if (pit != it->second.end())
		{
			pit->modelPrimitives.insert(pit->modelPrimitives.end(), prim.modelPrimitives.begin(), prim.modelPrimitives.end());
		}
		else
		{
			it->second.push_back(prim);
		}
	}
	return prim.graphicsID;
}

void PrimitiveProxy::RemoveModelPrimitives(Pass pass, HString graphicsID, HString modelPrimitiveName)
{
	auto pit = _allGraphicsPrimitives.find(pass);
	if (pit != _allGraphicsPrimitives.end())
	{
		auto git = std::find_if(pit->second.begin(), pit->second.end(), [graphicsID](GraphicsPrimitive& prim)
			{
				return prim.graphicsID == graphicsID;
			});
		if (git != pit->second.end())
		{
			auto mit = std::remove_if(git->modelPrimitives.begin(), git->modelPrimitives.end(), [modelPrimitiveName](ModelPrimitive& prim)
				{
					return modelPrimitiveName == prim.modelPrimitiveName;
				});
			if (mit != git->modelPrimitives.end())
			{
				git->modelPrimitives.erase(mit);
			}
		}
	}
}
