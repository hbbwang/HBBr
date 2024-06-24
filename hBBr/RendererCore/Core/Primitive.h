#pragma once
#include <vector>
#include <map>
#include "VertexFactory.h"
#include "Pass/PassType.h"
#include "Component/Transform.h"
#include "Texture2D.h"
#include "Pipeline.h"
#include "Shader.h"

//从Shader源码里获取到的Parameter信息
struct MaterialParameterInfo
{
	MPType type;
	HString name, ui;
	HString group = "Default";
	//shader uniform buffer 以vec4数组储存,这个索引用来读取数组对应的vec4
	uint32_t arrayIndex = 0;
	//配合上面索引获取到的vec4,该索引用来表示该参数处于vec4的什么位置
	uint32_t vec4Index = 0;
	//参数默认值
	std::vector<float> value;
};

//从Shader源码里获取到的Texture信息
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
	bool						bNeedUpdate = true;

	bool						bActive = true;

	HString						modelPrimitiveName;

	VertexFactory::VertexInput  vertexInput;

	glm::vec3					boundingBox_min = glm::vec3(0, 0, 0);

	glm::vec3					boundingBox_max = glm::vec3(0, 0, 0);

	Transform*					transform = nullptr;

	HString						matSocketName="";

	//用于排序
	int							priority = 0;

	std::vector<float>			vertexData;

	std::vector<uint32_t>		vertexIndices;

	uint64_t					vbPos = UINT64_MAX;

	uint64_t					vbSize = 0;

	uint64_t					ibPos = UINT64_MAX;

	uint64_t					ibSize = 0;

	std::vector<class VulkanRenderer*> rendererFrom;

	void SetActive(bool newActive)
	{
		if (newActive != bActive)
			bNeedUpdate = true;
		bActive = newActive;
	}
};

class MaterialPrimitive
{
	friend class Material;
public:
	MaterialPrimitive()
	{
		inputLayout = {};
		graphicsIndex = {};
		int priority = 0;
		HString graphicsName = "";
		Pass passUsing = Pass::OpaquePass;
		uniformBufferSize = 0;
		uniformBuffer.clear();
		_paramterInfos.clear();
		_textureInfos.clear();
		textures.clear();
		_samplers.clear();
		VkDescriptorSet _descriptorSet_tex = {};
		_needUpdateDescriptorSet_tex = true;
	}
	//Get material instance texture descriptor set.
	//If material instance number of textures is 0,it will return nullptr.
	HBBR_API HBBR_INLINE VkDescriptorSet GetDescriptorSet() {
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

	HBBR_API HBBR_INLINE const bool NeedUpdateTexture()
	{
		bool result = _needUpdateDescriptorSet_tex;
		if (result)
		{
			_needUpdateDescriptorSet_tex = false;
		}
		return result;
	}

	HBBR_API HBBR_INLINE std::vector<class Texture2D*> GetTextures() {
		return textures;
	}

	HBBR_API HBBR_INLINE std::vector<VkSampler> GetSamplers() {
		return _samplers;
	}

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API float GetScalarParameter(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec2 GetVector2Parameter(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec3 GetVector3Parameter(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec4 GetVector4Parameter(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	HBBR_API void SetScalarParameter(HString name, float value);

	HBBR_API void SetVec2Parameter(HString name, glm::vec2 value);

	HBBR_API void SetVec3Parameter(HString name, glm::vec3 value);

	HBBR_API void SetVec4Parameter(HString name, glm::vec4 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetScalarParameter(int arrayIndex, int vec4Index, float value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec2Parameter(int arrayIndex, int vec4Index, glm::vec2 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec3Parameter(int arrayIndex, int vec4Index, glm::vec3 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec4Parameter(int arrayIndex, int vec4Index, glm::vec4 value);

	HBBR_API void SetTextureSampler(int index, VkSampler sampler);

	HBBR_API void SetTextureSampler(HString textureName, VkSampler sampler);

	HBBR_API void SetTexture(int index, class Texture2D* newTexture);

	HBBR_API void SetTexture(HString textureName, class Texture2D* newTexture);

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

	std::vector<MaterialParameterInfo> _paramterInfos;

	std::vector<MaterialTextureInfo> _textureInfos;
private:

	//纹理贴图
	std::vector<class Texture2D*> textures;

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

	static void GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim ,HString vsFullName = "",HString psFullName = "");

	static void GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim, std::weak_ptr<ShaderCache> vs, std::weak_ptr<ShaderCache> ps);

	static void RemoveMaterialPrimitive(Pass pass, MaterialPrimitive* prim);

	static void AddModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim, class VulkanRenderer* renderer);

	static void RemoveModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim, class VulkanRenderer* renderer);

	inline static std::vector<std::vector<MaterialPrimitive*>> &GetAllMaterialPrimitiveArray() {
		return _allGraphicsPrimitives;
	}

	inline static std::vector<MaterialPrimitive*> *GetMaterialPrimitives(uint32_t index) {
		if (_allGraphicsPrimitives.size() > 0)
			return &_allGraphicsPrimitives[index];
		else
			return nullptr;
	}

	inline static std::vector<ModelPrimitive*>* GetModelPrimitives(MaterialPrimitive* index, class VulkanRenderer* renderer) {
		auto it = _allModelPrimitives.find(index);
		if (it != _allModelPrimitives.end())
			return &it->second[renderer];
		else
			return nullptr;
	}

private:

	static std::vector<std::vector<MaterialPrimitive*>> _allGraphicsPrimitives;

	static std::map<MaterialPrimitive*, std::map<class VulkanRenderer*, std::vector<ModelPrimitive*>>> _allModelPrimitives;

};