#pragma once
#include <vector>
#include <map>
#include "VertexFactory.h"
#include "Pass/PassType.h"
#include "Component/Transform.h"
#include "Texture.h"
#include "Pipeline.h"
#include "Shader.h"

struct MaterialParameterInfo
{
	MPType type;
	HString name, ui;
	//shader uniform buffer 以vec4数组储存,这个索引用来读取数组对应的vec4
	uint32_t arrayIndex = 0;
	//配合上面索引获取到的vec4,该索引用来表示该参数处于vec4的什么位置
	uint32_t vec4Index = 0;

	std::vector<float> value;
};

struct MaterialTextureInfo
{
	MTType type;
	MSAddress samplerAddress = MSAddress::Wrap;
	MSFilter samplerFilter = MSFilter::Linear;
	HString name, ui;
	uint32_t index = 0;
	HString value;
};

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

	uint64_t					vbSize = 0;

	uint64_t					ibPos = UINT64_MAX;

	uint64_t					ibSize = 0;
};

class MaterialPrimitive
{
	friend class Material;
public:

	//Get material instance texture descriptor set.
	//If material instance number of textures is 0,it will return NULL.
	HBBR_INLINE VkDescriptorSet GetDescriptorSet() {
		auto manager = VulkanManager::GetManager();
		if (textures.size() > 0)
		{
			if (_descriptorSet_tex == VK_NULL_HANDLE)
			{
				//Create DescriptorSet
				manager->AllocateDescriptorSet(manager->GetDescriptorPool(), manager->GetImageDescriptorSetLayout(), _descriptorSet_tex);
				_needUpdateDescriptorSet_tex = true;
			}
			return _descriptorSet_tex;
		}
		else
		{
			return VK_NULL_HANDLE;
		}
	}

	HBBR_INLINE const bool NeedUpdateTexture()
	{
		bool result = _needUpdateDescriptorSet_tex;
		if (result)
		{
			_needUpdateDescriptorSet_tex = false;
		}
		return result;
	}

	HBBR_INLINE std::vector<class Texture*> GetTextures() {
		return textures;
	}

	HBBR_INLINE std::vector<VkSampler> GetSamplers() {
		return _samplers;
	}

	void SetTextureSampler(int index, VkSampler sampler);

	void SetTextureSampler(HString textureName, VkSampler sampler);

	void SetTexture(int index, class Texture* newTexture);

	void SetTexture(HString textureName, class Texture* newTexture);

	~MaterialPrimitive()
	{
		if (_descriptorSet_tex != VK_NULL_HANDLE)
		{
			//std::vector<VkDescriptorSet> sets = { _descriptorSet_tex };
			//VulkanManager::GetManager()->FreeDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), sets);
			_descriptorSet_tex = VK_NULL_HANDLE;
		}
	}

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

	Pass passUsing;

	//参数
	//Shader参数
	std::vector<glm::vec4> uniformBuffer;

	uint64_t		   uniformBufferSize = 0;

	//变体开关
	uint64_t varients = 0;

	std::vector<MaterialParameterInfo> _paramterInfos;

	std::vector<MaterialTextureInfo> _textureInfos;
private:

	//纹理贴图
	std::vector<class Texture*> textures;

	//采样器选择
	std::vector<VkSampler> _samplers;

	//Image DescriptorSet
	VkDescriptorSet _descriptorSet_tex;

	bool _needUpdateDescriptorSet_tex;

};

class PrimitiveProxy
{
public:

	static void AddMaterialPrimitive( MaterialPrimitive* prim);

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