#pragma once
#include <vector>
#include "VertexFactory.h"

class Primitive
{
public:
	std::vector<VertexFactory::VertexData> _vertexData;
	std::vector <uint32_t> _vertexIndex;
};
