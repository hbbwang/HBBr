#pragma once
#include <vector>
#include <map>
#include "VertexFactory.h"
#include "Pass/PassType.h"
#include "Component/Transform.h"
#include "Texture.h"
#include "Pipeline.h"

//每个面的数据
struct ModelPrimitive
{
	HString						modelPrimitiveName;

	VertexFactory::VertexInput  vertexInput;

	glm::vec3					boundingBox_min = glm::vec3(0, 0, 0);

	glm::vec3					boundingBox_max = glm::vec3(0, 0, 0);

	Transform*					transform = NULL;

	HString						matSocketName="";

	//用于排序
	int							priority = 0;

	std::vector<float>			vertexData;

	std::vector<uint32_t>		vertexIndices;

	uint64_t					vbPos = UINT64_MAX;

	uint64_t					vbSize = UINT64_MAX;

	uint64_t					ibPos = UINT64_MAX;

	uint64_t					ibSize = UINT64_MAX;
};

struct MaterialPrimitive
{
	//Graphics用的什么vs
	HString vsShader = "BasePassTemplate";

	//Graphics用的什么ps
	HString psShader = "BasePassTemplate";

	//顶点输入
	VertexInputLayout	inputLayout;

	PipelineIndex	graphicsIndex;

	//用于排序
	int priority = 0;

	//其实就是材质名字
	HString graphicsName;

	Pass pass;

	//参数

	//纹理贴图
	std::vector<Texture*> textures;

	//Shader参数
	std::vector<float> uniformBuffer;

	//变体开关
	uint64_t varients = 0;
};

class PrimitiveProxy
{
public:

	static void AddMaterialPrimitive(Pass pass, MaterialPrimitive* prim);

	static void GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim);

	static void RemoveMaterialPrimitive(Pass pass, MaterialPrimitive* prim);

	static void AddModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim);

	static void RemoveModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim);

	inline static std::vector<std::vector<MaterialPrimitive*>> &GetAllMaterialPrimitiveArray() {
		return _allGraphicsPrimitives;
	}

	inline static std::vector<MaterialPrimitive*> &GetMaterialPrimitives(uint32_t index) {
		return _allGraphicsPrimitives[index];
	}

	inline static std::map<MaterialPrimitive*, std::vector<ModelPrimitive*>> &GetAllModelPrimitiveArray() {
		return _allModelPrimitives;
	}

	inline static std::vector<ModelPrimitive*> &GetModelPrimitives(MaterialPrimitive* index) {
		return _allModelPrimitives[index];
	}

private:

	static std::vector<std::vector<MaterialPrimitive*>> _allGraphicsPrimitives;

	static std::map<MaterialPrimitive*, std::vector<ModelPrimitive*>> _allModelPrimitives;

};