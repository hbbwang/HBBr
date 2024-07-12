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
	HString group = "Default";
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

	class VulkanRenderer* renderer = nullptr;

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
		uniformBufferSize_vs = 0;
		uniformBufferSize_ps = 0;
		uniformBuffer_vs.clear();
		uniformBuffer_ps.clear();
		_paramterInfos_vs.clear();
		_paramterInfos_ps.clear();
		_textureInfos.clear();
		textures.clear();
		_samplers.clear();
	}

	/*HBBR_INLINE void Reset()
	{
		auto manager = VulkanManager::GetManager();
		const int descriptorSetCount = (int)_descriptorSet_tex.size();
		if (descriptorSetCount != manager->GetSwapchainBufferCount())
		{
			_needUpdateDescriptorSet_tex.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
			_descriptorSet_tex.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
			for (int i = 0; i < _descriptorSet_tex.size(); i++)
			{
				manager->AllocateDescriptorSet(manager->GetDescriptorPool(), manager->GetImageDescriptorSetLayout(), _descriptorSet_tex[i]);
				_needUpdateDescriptorSet_tex[i] = 1;
			}
		}
	}*/

	HBBR_API HBBR_INLINE std::vector<class Texture2D*> GetTextures() {
		return textures;
	}

	HBBR_API HBBR_INLINE std::vector<VkSampler> GetSamplers() {
		return _samplers;
	}

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API float GetScalarParameter_VS(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec2 GetVector2Parameter_VS(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec3 GetVector3Parameter_VS(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec4 GetVector4Parameter_VS(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API float GetScalarParameter_PS(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec2 GetVector2Parameter_PS(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec3 GetVector3Parameter_PS(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API glm::vec4 GetVector4Parameter_PS(HString name, int* arrayIndex = nullptr, int* vec4Index = nullptr);

	HBBR_API void SetScalarParameter_PS(HString name, float value);

	HBBR_API void SetVec2Parameter_PS(HString name, glm::vec2 value);

	HBBR_API void SetVec3Parameter_PS(HString name, glm::vec3 value);

	HBBR_API void SetVec4Parameter_PS(HString name, glm::vec4 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetScalarParameter_PS(int arrayIndex, int vec4Index, float value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec2Parameter_PS(int arrayIndex, int vec4Index, glm::vec2 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec3Parameter_PS(int arrayIndex, int vec4Index, glm::vec3 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec4Parameter_PS(int arrayIndex, int vec4Index, glm::vec4 value);

	HBBR_API void SetScalarParameter_VS(HString name, float value);

	HBBR_API void SetVec2Parameter_VS(HString name, glm::vec2 value);

	HBBR_API void SetVec3Parameter_VS(HString name, glm::vec3 value);

	HBBR_API void SetVec4Parameter_VS(HString name, glm::vec4 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetScalarParameter_VS(int arrayIndex, int vec4Index, float value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec2Parameter_VS(int arrayIndex, int vec4Index, glm::vec2 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec3Parameter_VS(int arrayIndex, int vec4Index, glm::vec3 value);

	//arrayIndex和arrayCount可选，是用来快速定位修改参数用的
	HBBR_API void SetVec4Parameter_VS(int arrayIndex, int vec4Index, glm::vec4 value);

	HBBR_API void SetTextureSampler(int index, VkSampler sampler);

	HBBR_API void SetTextureSampler(HString textureName, VkSampler sampler);

	HBBR_API void SetTexture(int index, class Texture2D* newTexture);

	HBBR_API void SetTexture(HString textureName, class Texture2D* newTexture);

	~MaterialPrimitive()
	{
	}

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
	//PS
	std::vector<glm::vec4> uniformBuffer_ps;
	uint64_t uniformBufferSize_ps = 0;
	std::vector<MaterialParameterInfo> _paramterInfos_ps;

	//VS
	std::vector<glm::vec4> uniformBuffer_vs;
	uint64_t uniformBufferSize_vs = 0;
	std::vector<MaterialParameterInfo> _paramterInfos_vs;

	//Texs
	std::vector<MaterialTextureInfo> _textureInfos;

private:

	//纹理贴图
	std::vector<class Texture2D*> textures;

	//采样器选择
	std::vector<VkSampler> _samplers;

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