#pragma once
#include <vector>
#include <map>
#include "VertexFactory.h"
#include "Pass/PassType.h"
struct ModelPrimitive
{
	HString						name;

	std::vector<VertexFactory::VertexInput>	vertexInputs;

	std::vector<std::vector<float>>			vertexData;

	std::vector<std::vector<uint32_t>>		vertexIndices;

	std::vector<VertexInputLayout>			vertexInputLayouts;

	glm::vec3					boundingBox_min = glm::vec3(0, 0, 0);

	glm::vec3					boundingBox_max = glm::vec3(0, 0, 0);

	glm::mat4					worldMatrix = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	bool	bBasePass;
};

class PrimitiveProxy
{
public:

	static void AddModelPrimitive(Pass pass, ModelPrimitive model);

	static std::vector<ModelPrimitive> GetModelPrimitives(Pass pass) 
	{
		return _allModelPrimitives[pass];
	}

private:

	static std::map<Pass, std::vector<ModelPrimitive>> _allModelPrimitives;

};