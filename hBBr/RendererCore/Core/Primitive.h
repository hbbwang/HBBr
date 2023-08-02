#pragma once
#include <vector>
#include <map>
#include "VertexFactory.h"
#include "Pass/PassType.h"

//每个面的数据
struct ModelPrimitive
{
	HString						objName;

	VertexFactory::VertexInput  vertexInput;

	std::vector<float>			vertexData;

	std::vector<uint32_t>		vertexIndices;

	glm::vec3					boundingBox_min = glm::vec3(0, 0, 0);

	glm::vec3					boundingBox_max = glm::vec3(0, 0, 0);

	glm::mat4					worldMatrix = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	//用于排序
	int							priority = 0;
};

struct GraphicsPrimitive
{
	//一共有多少个面在这个Graphics里
	std::vector<ModelPrimitive>				modelPrimitives;
	
	//Graphics用的什么vs
	HString vsShader;

	//Graphics用的什么ps
	HString psShader;

	//其实就是材质名字
	HString graphicsName;

	//用于排序
	int priority = 0;

	HString graphicsID;
	
	VertexInputLayout			inputLayout;
};


class PrimitiveProxy
{
public:

	static void AddGraphicsPrimitives(Pass pass, HString vsShader , HString psShader , GraphicsPrimitive prim);

	static std::vector<GraphicsPrimitive> GetGraphicsPrimitives(Pass pass)
	{
		return _allGraphicsPrimitives[pass];
	}

private:

	static std::map<Pass, std::vector<GraphicsPrimitive>> _allGraphicsPrimitives;

};