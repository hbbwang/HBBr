#pragma once
#include <vector>
#include <map>
#include "VertexFactory.h"
#include "Pass/PassType.h"
#include "Component/Transform.h"
#include "Texture2D.h"
#include "Pipeline.h"
#include "Shader.h"
#include "DescriptorSet.h"

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
	uint8_t						bNeedUpdate = true;

	//uint8_t						bActive = true;

	HString						modelPrimitiveName;

	VertexFactory::VertexInput  vertexInput;

	glm::vec3					boundingBox_min = glm::vec3(0, 0, 0);

	glm::vec3					boundingBox_max = glm::vec3(0, 0, 0);

	Transform*					transform = nullptr;

	HString						matSocketName="";

	//用于排序
	int							priority = 0;

	uint64_t					vbSize = 0;

	uint64_t					ibSize = 0;

	std::vector<float>			vertexData;

	std::vector<uint32_t>		vertexIndices;

	std::vector<class VulkanRenderer*> rendererFrom;

	class VulkanRenderer* renderer = nullptr;

	//void SetActive(bool newActive)
	//{
	//	if ((uint8_t)newActive != bActive)
	//		bNeedUpdate = 1;
	//	bActive = (uint8_t)newActive;
	//}
};

struct MaterialPrimitiveGroup
{
	std::vector<ModelPrimitive*> prims;

	VkDeviceSize	vbWholeSize = 0;

	VkDeviceSize	ibWholeSize = 0;

	class VulkanRenderer* renderer = nullptr;

	std::shared_ptr<class DescriptorSet> descriptorSet_uniformBufferVS;

	std::shared_ptr<class DescriptorSet> descriptorSet_uniformBufferPS;

	std::vector<VkDescriptorSet>	descriptorSet_texture;

	class MaterialPrimitive* primFrom = nullptr;

	uint8_t needCopyDataVS = 1;

	uint8_t needCopyDataPS = 1;

	std::vector<uint8_t> needUpdateTextures;

	//
	void ResetDecriptorSet(uint8_t numTextures, bool& bNeedUpdateVSUniformBuffer, bool& bNeedUpdatePSUniformBuffer, bool& bNeedUpdateTextures);

	void UpdateDecriptorSet(bool bNeedUpdateVSUniformBuffer, bool bNeedUpdatePSUniformBuffer, bool bNeedUpdateTextures);

	MaterialPrimitiveGroup() {
		needCopyDataVS = 1;
		needCopyDataPS = 1;
	}
};

class MaterialPrimitive
{
	friend class Material;
	friend class PrimitiveProxy;
public:
	MaterialPrimitive()
	{
		_inputLayout = {};
		_graphicsIndex = {};
		int priority = 0;
		HString graphicsName = "";
		Pass passUsing = Pass::OpaquePass;
		_uniformBufferSize_vs = 0;
		_uniformBufferSize_ps = 0;
		_uniformBuffer_vs.clear();
		_uniformBuffer_ps.clear();
		_paramterInfos_vs.clear();
		_paramterInfos_ps.clear();
		_textureInfos.clear();
		_textures.clear();
		_samplers.clear();
		_materialPrimitiveGroups.clear();
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

	HBBR_API HBBR_INLINE std::vector<std::shared_ptr<Texture2D>> GetTextures() {
		return _textures;
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

	HBBR_API void SetTexture(int index, std::shared_ptr<class Texture2D> newTexture);

	HBBR_API void SetTexture(HString textureName, std::shared_ptr<class Texture2D> newTexture);

	HBBR_API void UpdateUniformBufferVS();

	HBBR_API void UpdateUniformBufferPS();

	HBBR_API void UpdateTextures();

	~MaterialPrimitive()
	{
	}

	//顶点输入
	VertexInputLayout	_inputLayout;

	//管线索引
	PipelineIndex		_graphicsIndex;

	//用于排序
	int					_priority = 0;

	//其实就是材质名字
	HString				_graphicsName;

	//使用的是哪个pass
	Pass				_passUsing;

	//-----------------------参数 Shader参数

	//PS
	std::vector<glm::vec4>				_uniformBuffer_ps;
	uint64_t							_uniformBufferSize_ps = 0;
	std::vector<MaterialParameterInfo>	_paramterInfos_ps;

	//VS
	std::vector<glm::vec4>				_uniformBuffer_vs;
	uint64_t							_uniformBufferSize_vs = 0;
	std::vector<MaterialParameterInfo>	_paramterInfos_vs;

	//Textures
	std::vector<MaterialTextureInfo>	_textureInfos;

private:

	//纹理贴图
	std::vector<std::shared_ptr<Texture2D>>		_textures;

	//采样器选择
	std::vector<VkSampler>				_samplers;

	std::map<VulkanRenderer*, MaterialPrimitiveGroup>	_materialPrimitiveGroups;

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

	inline static MaterialPrimitiveGroup* GetMaterialPrimitiveGroup(MaterialPrimitive* mat, class VulkanRenderer* renderer) {
		if (mat)
		{
			auto result = mat->_materialPrimitiveGroups.find(renderer);
			if(result != mat->_materialPrimitiveGroups.end())
				return &result->second;
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	static void ClearAll();

private:

	static std::vector<std::vector<MaterialPrimitive*>> _allGraphicsPrimitives;

};