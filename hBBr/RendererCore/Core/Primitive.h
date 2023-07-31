#pragma once
#include <vector>
#include "VertexFactory.h"

class MeshPrimitive
{
public:
	VertexFactory::VertexInput	_vertexInput;
	std::vector<uint32_t>		_vertexIndices;
};
