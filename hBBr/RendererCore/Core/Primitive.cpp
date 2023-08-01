#pragma once
#include "Primitive.h"

std::map<Pass, std::vector<ModelPrimitive>> PrimitiveProxy::_allModelPrimitives;

void PrimitiveProxy::AddModelPrimitive(Pass pass, ModelPrimitive model)
{
	auto it = _allModelPrimitives.find(pass);
	if (it == _allModelPrimitives.end())
	{
		_allModelPrimitives.emplace(std::make_pair(pass, std::vector<ModelPrimitive>({ model })));
	}
	else
	{
		it->second.push_back(model);
	}
}
