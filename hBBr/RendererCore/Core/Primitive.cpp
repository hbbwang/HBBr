#include "Primitive.h"
#include "Shader.h"
#include "VulkanRenderer.h"
#include "Model.h"

std::vector<std::vector<MaterialPrimitive*>> PrimitiveProxy::_allGraphicsPrimitives;

void PrimitiveProxy::AddMaterialPrimitive(MaterialPrimitive* prim)
{
	if ((uint32_t)PrimitiveProxy::_allGraphicsPrimitives.size() != (uint32_t)Pass::MaxNum)
	{
		_allGraphicsPrimitives.resize((uint32_t)Pass::MaxNum);
	}
	auto& graphicsPrimitive = _allGraphicsPrimitives[(uint32_t)prim->_passUsing];
	if (graphicsPrimitive.capacity() <= graphicsPrimitive.size())
	{
		if (graphicsPrimitive.capacity() < 200)
			graphicsPrimitive.reserve(graphicsPrimitive.capacity() + 20);
		else
			graphicsPrimitive.reserve(graphicsPrimitive.capacity() * 2);
	}

	graphicsPrimitive.push_back(prim);
}

void PrimitiveProxy::RemoveMaterialPrimitive(Pass pass, MaterialPrimitive* prim)
{
	if ((uint32_t)PrimitiveProxy::_allGraphicsPrimitives.size() != (uint32_t)Pass::MaxNum)
	{
		_allGraphicsPrimitives.resize((uint32_t)Pass::MaxNum);
	}
	int index = (uint32_t)pass;
	auto it = std::find(_allGraphicsPrimitives[index].begin(), _allGraphicsPrimitives[index].end(), prim);
	if (it != _allGraphicsPrimitives[index].end())
	{
		for (auto& matGroup : (*it)->_materialPrimitiveGroups)
		{
			delete matGroup.second;
			matGroup.second = nullptr;
		}
		(*it)->_materialPrimitiveGroups.clear();
		_allGraphicsPrimitives[index].erase(it);
	}
}

void PrimitiveProxy::GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim, std::string vsFullName, std::string psFullName)
{
	if (vsFullName.length() <= 1 || psFullName.length() <= 1)
	{
		prim->_graphicsIndex = PipelineIndex::GetPipelineIndex(
			Shader::_vsShader[prim->_graphicsIndex.GetVSShaderName() + "@" + StringTool::FromUInt(prim->_graphicsIndex.GetVSVarient())],
			Shader::_psShader[prim->_graphicsIndex.GetPSShaderName() + "@" + StringTool::FromUInt(prim->_graphicsIndex.GetPSVarient())]);
	}
	else
	{
		prim->_graphicsIndex = PipelineIndex::GetPipelineIndex(
			Shader::_vsShader[vsFullName],
			Shader::_psShader[psFullName]);
	}
}

void PrimitiveProxy::GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim, std::weak_ptr<ShaderCache> vs, std::weak_ptr<ShaderCache> ps)
{
	prim->_graphicsIndex = PipelineIndex::GetPipelineIndex(
		vs,
		ps);
}

void PrimitiveProxy::AddModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim, class VulkanRenderer* renderer)
{
	if (!mat)
		return;

	prim->vertexData = prim->faceData->vertexData.GetData(Shader::_vsShader[mat->_graphicsIndex.GetVSShaderFullName()]->header.vertexInput);
	prim->vertexIndices = prim->faceData->vertexData.vertexIndices;

	MaterialPrimitiveGroup* matGroup = nullptr;
	auto it = mat->_materialPrimitiveGroups.find(renderer);
	if (it == mat->_materialPrimitiveGroups.end())
	{
		matGroup = new MaterialPrimitiveGroup();
		matGroup->prims = std::vector<ModelPrimitive*>();
		matGroup->primFrom = mat;
		matGroup->renderer = renderer;
		mat->_materialPrimitiveGroups.emplace(renderer, matGroup);
	}
	else
	{
		matGroup = it->second;
	}

	if (matGroup->prims.capacity() <= matGroup->prims.size())
	{
		if (matGroup->prims.capacity() < 200)
			matGroup->prims.reserve(matGroup->prims.capacity() + 20);
		else
			matGroup->prims.reserve(matGroup->prims.capacity() * 2);
	}

	prim->bNeedUpdate = true;
	prim->vbSize = prim->vertexData.size()	* sizeof(float);
	prim->ibSize = prim->vertexIndices.size()	* sizeof(uint32_t);
	matGroup->prims.push_back(prim);
	matGroup->vbWholeSize += prim->vbSize;
	matGroup->ibWholeSize += prim->ibSize;
	//modelPrim.vbUpdatePos = std::min(modelPrim.vbUpdatePos, prim->vbPos);
	//modelPrim.ibUpdatePos = std::min(modelPrim.ibUpdatePos, prim->ibPos);

}

void PrimitiveProxy::RemoveModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim, class VulkanRenderer* renderer)
{
	if(prim)
	{
		auto rit = mat->_materialPrimitiveGroups.find(renderer);
		if (rit != mat->_materialPrimitiveGroups.end())
		{
			auto pit = std::find(rit->second->prims.begin(), rit->second->prims.end(), prim);
			if (pit != rit->second->prims.end())
			{
				//是否是最后一个对象,如果不是，需要标记下一个对象需要更新Buffer
				bool isTheLastItem = false;
				if (rit->second->prims.back() != *pit)
				{
					(*(pit + 1))->bNeedUpdate = true;
				}
				else
				{
					isTheLastItem = true;
				}

				rit->second->vbWholeSize -= (*pit)->vbSize;
				rit->second->ibWholeSize -= (*pit)->ibSize;

				rit->second->prims.erase(pit);
				if (rit->second->prims.size() > 0 && isTheLastItem)
					rit->second->prims.back()->bNeedUpdate = true;
			}
		}
		
	}
}

void PrimitiveProxy::ClearAll()
{
	_allGraphicsPrimitives.clear();
}

void MaterialPrimitive::SetTexture(int index, std::shared_ptr<class Texture2D> newTexture)
{
	UpdateTextures();
	_textures[index] = newTexture;
}

void MaterialPrimitive::SetTexture(std::string textureName, std::shared_ptr<class Texture2D>newTexture)
{
	auto it = std::find_if(_textureInfos.begin(), _textureInfos.end(), [textureName](MaterialTextureInfo& info) {
		return StringTool::IsEqual(info.name, textureName, false);
	});
	if (it != _textureInfos.end())
	{
		UpdateTextures();
		SetTexture(it->index, newTexture);
	}
}

void MaterialPrimitive::UpdateUniformBufferVS()
{
	for (auto& i : _materialPrimitiveGroups)
	{
		i.second->needCopyDataVS = 1;
	}
}

void MaterialPrimitive::UpdateUniformBufferPS()
{
	for (auto& i : _materialPrimitiveGroups)
	{
		i.second->needCopyDataPS = 1;
	}
}

void MaterialPrimitive::UpdateTextures()
{
	for (auto& i : _materialPrimitiveGroups)
	{
		auto& updateContent = i.second->needUpdateTextures;
		const auto updateCount = updateContent.size();
		if (updateCount > 0)
		{
			memset(updateContent.data(), 1, updateCount * sizeof(uint8_t));
		}
	}
}

MaterialPrimitive::MaterialPrimitive()
{
	_graphicsIndex = {};
	int priority = 0;
	std::string graphicsName = "";
	Pass passUsing = Pass::BasePass;
	_uniformBufferSize_vs = 0;
	_uniformBufferSize_ps = 0;
	_inputLayout.reset(new VertexInputLayout);
}


MaterialPrimitive::~MaterialPrimitive()
{
	_inputLayout.reset();
	_graphicsIndex = {};
	Pass passUsing = Pass::BasePass;
	_uniformBuffer_vs.clear();
	_uniformBuffer_ps.clear();
	_paramterInfos_vs.clear();
	_paramterInfos_ps.clear();
	_textureInfos.clear();
	_textures.clear();
	_samplers.clear();
	for (auto& i : _materialPrimitiveGroups)
	{
		delete i.second;
		i.second = nullptr;
	}
	_materialPrimitiveGroups.clear();
}

float MaterialPrimitive::GetScalarParameter_VS(std::string name, int* arrayIndex, int* vec4Index)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		return _uniformBuffer_vs[it->arrayIndex][it->vec4Index];
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return 0;
}

glm::vec2 MaterialPrimitive::GetVector2Parameter_VS(std::string name, int* arrayIndex, int* vec4Index)
{
	glm::vec2 result = glm::vec2(0);
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = _uniformBuffer_vs[it->arrayIndex][it->vec4Index];
		result.y = _uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec3 MaterialPrimitive::GetVector3Parameter_VS(std::string name, int* arrayIndex, int* vec4Index)
{
	glm::vec3 result = glm::vec3(0);
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = _uniformBuffer_vs[it->arrayIndex][it->vec4Index];
		result.y = _uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1];
		result.z = _uniformBuffer_vs[it->arrayIndex][it->vec4Index + 2];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec4 MaterialPrimitive::GetVector4Parameter_VS(std::string name, int* arrayIndex, int* vec4Index)
{
	glm::vec4 result = glm::vec4(0);
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = _uniformBuffer_vs[it->arrayIndex][it->vec4Index];
		result.y = _uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1];
		result.z = _uniformBuffer_vs[it->arrayIndex][it->vec4Index + 2];
		result.w = _uniformBuffer_vs[it->arrayIndex][it->vec4Index + 3];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

float MaterialPrimitive::GetScalarParameter_PS(std::string name, int* arrayIndex, int* vec4Index)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		if (arrayIndex != nullptr) 
		{
			*arrayIndex = (int)it->arrayIndex;
		}
		if (vec4Index != nullptr) 
		{
			*vec4Index = (int)it->vec4Index;
		}
		return _uniformBuffer_ps[it->arrayIndex][it->vec4Index];
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return 0;
}

glm::vec2 MaterialPrimitive::GetVector2Parameter_PS(std::string name, int* arrayIndex, int* vec4Index)
{
	glm::vec2 result = glm::vec2(0);
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		if (arrayIndex != nullptr) 
		{
			*arrayIndex = (int)it->arrayIndex;
		}
		if (vec4Index != nullptr) 
		{
			*vec4Index = (int)it->vec4Index;
		}
		result.x = _uniformBuffer_ps[it->arrayIndex][it->vec4Index];
		result.y = _uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec3 MaterialPrimitive::GetVector3Parameter_PS(std::string name, int* arrayIndex, int* vec4Index)
{
	glm::vec3 result = glm::vec3(0);
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		if (arrayIndex != nullptr) 
		{
			*arrayIndex = (int)it->arrayIndex;
		}
		if (vec4Index != nullptr) 
		{
			*vec4Index = (int)it->vec4Index;
		}
		result.x = _uniformBuffer_ps[it->arrayIndex][it->vec4Index];
		result.y = _uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1];
		result.z = _uniformBuffer_ps[it->arrayIndex][it->vec4Index + 2];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec4 MaterialPrimitive::GetVector4Parameter_PS(std::string name, int* arrayIndex, int* vec4Index)
{
	glm::vec4 result = glm::vec4(0);
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		if (arrayIndex != nullptr) 
		{
			*arrayIndex = (int)it->arrayIndex;
		}
		if (vec4Index != nullptr)
		{
			*vec4Index = (int)it->vec4Index;
		}
		result.x = _uniformBuffer_ps[it->arrayIndex][it->vec4Index];
		result.y = _uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1];
		result.z = _uniformBuffer_ps[it->arrayIndex][it->vec4Index + 2];
		result.w = _uniformBuffer_ps[it->arrayIndex][it->vec4Index + 3];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

void MaterialPrimitive::SetScalarParameter_PS(std::string name, float value)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		UpdateUniformBufferPS();
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index] = value;
	}
}

void MaterialPrimitive::SetVec2Parameter_PS(std::string name, glm::vec2 value)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		UpdateUniformBufferPS();
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index] = value.x;
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1] = value.y;
	}
}

void MaterialPrimitive::SetVec3Parameter_PS(std::string name, glm::vec3 value)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		UpdateUniformBufferPS();
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index] = value.x;
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1] = value.y;
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index + 2] = value.z;
	}
}

void MaterialPrimitive::SetVec4Parameter_PS(std::string name, glm::vec4 value)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		UpdateUniformBufferPS();
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index] = value.x;
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1] = value.y;
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index + 2] = value.z;
		_uniformBuffer_ps[it->arrayIndex][it->vec4Index + 3] = value.w;
	}
}

void MaterialPrimitive::SetScalarParameter_PS(int arrayIndex, int vec4Index, float value)
{
	_uniformBuffer_ps[arrayIndex][vec4Index] = value;
	UpdateUniformBufferPS();
}

void MaterialPrimitive::SetVec2Parameter_PS(int arrayIndex, int vec4Index, glm::vec2 value)
{
	_uniformBuffer_ps[arrayIndex][vec4Index] = value.x;
	_uniformBuffer_ps[arrayIndex][vec4Index + 1] = value.y;
	UpdateUniformBufferPS();
}

void MaterialPrimitive::SetVec3Parameter_PS(int arrayIndex, int vec4Index, glm::vec3 value)
{
	_uniformBuffer_ps[arrayIndex][vec4Index] = value.x;
	_uniformBuffer_ps[arrayIndex][vec4Index + 1] = value.y;
	_uniformBuffer_ps[arrayIndex][vec4Index + 2] = value.z;
	UpdateUniformBufferPS();
}

void MaterialPrimitive::SetVec4Parameter_PS(int arrayIndex, int vec4Index, glm::vec4 value)
{
	_uniformBuffer_ps[arrayIndex][vec4Index] = value.x;
	_uniformBuffer_ps[arrayIndex][vec4Index + 1] = value.y;
	_uniformBuffer_ps[arrayIndex][vec4Index + 2] = value.z;
	_uniformBuffer_ps[arrayIndex][vec4Index + 3] = value.w;
	UpdateUniformBufferPS();
}

void MaterialPrimitive::SetScalarParameter_VS(std::string name, float value)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		UpdateUniformBufferVS();
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index] = value;
	}
}

void MaterialPrimitive::SetVec2Parameter_VS(std::string name, glm::vec2 value)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		UpdateUniformBufferVS();
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index] = value.x;
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1] = value.y;
	}
}

void MaterialPrimitive::SetVec3Parameter_VS(std::string name, glm::vec3 value)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		UpdateUniformBufferVS();
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index] = value.x;
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1] = value.y;
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index + 2] = value.z;
	}
}

void MaterialPrimitive::SetVec4Parameter_VS(std::string name, glm::vec4 value)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		UpdateUniformBufferVS();
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index] = value.x;
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1] = value.y;
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index + 2] = value.z;
		_uniformBuffer_vs[it->arrayIndex][it->vec4Index + 3] = value.w;
	}
}

void MaterialPrimitive::SetScalarParameter_VS(int arrayIndex, int vec4Index, float value)
{
	UpdateUniformBufferVS();
	_uniformBuffer_vs[arrayIndex][vec4Index] = value;
}

void MaterialPrimitive::SetVec2Parameter_VS(int arrayIndex, int vec4Index, glm::vec2 value)
{
	UpdateUniformBufferVS();;
	_uniformBuffer_vs[arrayIndex][vec4Index] = value.x;
	_uniformBuffer_vs[arrayIndex][vec4Index + 1] = value.y;
}

void MaterialPrimitive::SetVec3Parameter_VS(int arrayIndex, int vec4Index, glm::vec3 value)
{
	UpdateUniformBufferVS();
	_uniformBuffer_vs[arrayIndex][vec4Index] = value.x;
	_uniformBuffer_vs[arrayIndex][vec4Index + 1] = value.y;
	_uniformBuffer_vs[arrayIndex][vec4Index + 2] = value.z;
}

void MaterialPrimitive::SetVec4Parameter_VS(int arrayIndex, int vec4Index, glm::vec4 value)
{
	UpdateUniformBufferVS();
	_uniformBuffer_vs[arrayIndex][vec4Index] = value.x;
	_uniformBuffer_vs[arrayIndex][vec4Index + 1] = value.y;
	_uniformBuffer_vs[arrayIndex][vec4Index + 2] = value.z;
	_uniformBuffer_vs[arrayIndex][vec4Index + 3] = value.w;
}

void MaterialPrimitive::SetTextureSampler(int index, VkSampler sampler)
{
	UpdateTextures();
	_samplers[index] = sampler;
}

void MaterialPrimitive::SetTextureSampler(std::string textureName, VkSampler sampler)
{
	auto it = std::find_if(_textureInfos.begin(), _textureInfos.end(), [textureName](MaterialTextureInfo& info) {
		return StringTool::IsEqual(info.name,textureName, false);
		});
	if (it != _textureInfos.end())
	{
		UpdateTextures();
		SetTextureSampler(it->index, sampler);
	}
}

void MaterialPrimitiveGroup::ResetDecriptorSet(uint8_t numTextures, bool& bNeedUpdateVSUniformBuffer, bool& bNeedUpdatePSUniformBuffer, bool& bNeedUpdateTextures)
{
	auto vkManager = VulkanManager::GetManager();
	const auto frameIndex = renderer->GetSwapchain()->GetCurrentFrameIndex();

	if (!descriptorSet_uniformBufferVS)
	{
		descriptorSet_uniformBufferVS.reset(new DescriptorSet(renderer));
		descriptorSet_uniformBufferVS->CreateBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
		descriptorSet_uniformBufferVS->CreateBuffer(0, 32, VMA_MEMORY_USAGE_CPU_TO_GPU, true, false, primFrom->_graphicsName + "_Material_Ub_VS");
		descriptorSet_uniformBufferVS->BuildDescriptorSetLayout();
	}
	
	if (!descriptorSet_uniformBufferPS)
	{
		descriptorSet_uniformBufferPS.reset(new DescriptorSet(renderer));
		descriptorSet_uniformBufferPS->CreateBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT);
		descriptorSet_uniformBufferPS->CreateBuffer(0, 32, VMA_MEMORY_USAGE_CPU_TO_GPU, true, false, primFrom->_graphicsName + "_Material_Ub_PS");
		descriptorSet_uniformBufferPS->BuildDescriptorSetLayout();
	}

	if (primFrom->_uniformBufferSize_vs > 0)
	{
		//Update vs buffer
		if (primFrom->_uniformBufferSize_vs > descriptorSet_uniformBufferVS->GetBuffer(0)->GetBufferSize())
			bNeedUpdateVSUniformBuffer = descriptorSet_uniformBufferVS->GetBuffer(0)->Resize(primFrom->_uniformBufferSize_vs);
	}
	if (primFrom->_uniformBufferSize_ps > 0)
	{
		//Update ps buffer
		if (primFrom->_uniformBufferSize_ps > descriptorSet_uniformBufferPS->GetBuffer(0)->GetBufferSize())
			bNeedUpdatePSUniformBuffer = descriptorSet_uniformBufferPS->GetBuffer(0)->Resize(primFrom->_uniformBufferSize_ps);
	}
	//Update texture
	bool bNeedRecreate = false;
	if (vkManager->GetSwapchainBufferCount() != descriptorSet_texture.size())
	{
		descriptorSet_texture.resize(vkManager->GetSwapchainBufferCount());
		needUpdateTextures.resize(vkManager->GetSwapchainBufferCount());
		memset(needUpdateTextures.data(), 1, needUpdateTextures.size() * sizeof(uint8_t) );
		bNeedRecreate = true;
	}
	auto& dstex = descriptorSet_texture[frameIndex];
	if (primFrom->_textureInfos.size() > 0)
	{
		if (bNeedRecreate || dstex == VK_NULL_HANDLE || primFrom->_textureInfos.size() != numTextures)
		{
			if (dstex != VK_NULL_HANDLE)
				vkManager->FreeDescriptorSet(vkManager->GetDescriptorPool(), dstex);
			dstex = VK_NULL_HANDLE;
			vkManager->AllocateDescriptorSet(vkManager->GetDescriptorPool(), PipelineManager::GetDescriptorSetLayout_TextureSamplerVSPS(numTextures), dstex);
			bNeedUpdateTextures = true;
		}
	}
	else
	{
		//没打算允许Vulkan手动释放符集的功能，所以不做释放，如果已经初始化，那就把数量降低到1，并改为绑定小图，VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
		if (dstex != VK_NULL_HANDLE && numTextures != 1)
		{
			if (dstex != VK_NULL_HANDLE)
				vkManager->FreeDescriptorSet(vkManager->GetDescriptorPool(), dstex);
			dstex = VK_NULL_HANDLE;
			vkManager->AllocateDescriptorSet(vkManager->GetDescriptorPool(), PipelineManager::GetDescriptorSetLayout_TextureSamplerVSPS(numTextures), dstex);
			bNeedUpdateTextures = true;
		}
	}
}

void MaterialPrimitiveGroup::UpdateDecriptorSet(bool bNeedUpdateVSUniformBuffer, bool bNeedUpdatePSUniformBuffer, bool bNeedUpdateTextures)
{
	auto* vkManager = VulkanManager::GetManager();
	const auto frameIndex = renderer->GetSwapchain()->GetCurrentFrameIndex();
	if (bNeedUpdateVSUniformBuffer)
		descriptorSet_uniformBufferVS->RefreshDescriptorSet(0);
	if (bNeedUpdatePSUniformBuffer)
		descriptorSet_uniformBufferPS->RefreshDescriptorSet(0);
	//Update vs uniform buffer
	if (primFrom->_uniformBufferSize_vs > 0)
	{
		if (needCopyDataVS)
		{
			needCopyDataVS = 0;
			descriptorSet_uniformBufferVS->BufferMapping(0, primFrom->_uniformBuffer_vs.data(), 0, primFrom->_uniformBufferSize_vs);
		}
		descriptorSet_uniformBufferVS->UpdateBufferDescriptorSet(0, 0, primFrom->_uniformBufferSize_vs);
	}
	//Update ps uniform buffer
	if (primFrom->_uniformBufferSize_ps > 0)
	{
		if (needCopyDataPS)
		{
			needCopyDataPS = 0;
			descriptorSet_uniformBufferPS->BufferMapping(0, primFrom->_uniformBuffer_ps.data(), 0, primFrom->_uniformBufferSize_ps);
		}
		descriptorSet_uniformBufferPS->UpdateBufferDescriptorSet(0, 0, primFrom->_uniformBufferSize_ps);
	}
	//Update vsps _textures
	if (descriptorSet_texture.size() > 0 && descriptorSet_texture[frameIndex] != VK_NULL_HANDLE)
	{
		if (needUpdateTextures[frameIndex] == 1)
		{
			needUpdateTextures[frameIndex] = 0;
			bNeedUpdateTextures = true;
		}
		if (bNeedUpdateTextures)
		{
			if (primFrom->_textureInfos.size() > 0)
			{
				vkManager->UpdateTextureDescriptorSet(descriptorSet_texture[frameIndex], primFrom->GetTextures(), primFrom->GetSamplers());
			}
			else
			{
				vkManager->UpdateTextureDescriptorSet(descriptorSet_texture[frameIndex], { Texture2D::GetSystemTexture("Black") }, { Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp) });
			}
		}
	}
}

MaterialPrimitiveGroup::MaterialPrimitiveGroup()
{
	needCopyDataVS = 1;
	needCopyDataPS = 1;
	vbWholeSize = 0;
	ibWholeSize = 0;
}

MaterialPrimitiveGroup::~MaterialPrimitiveGroup()
{
	prims.clear();
	renderer = nullptr;
	descriptorSet_uniformBufferVS.reset();
	descriptorSet_uniformBufferPS.reset();
	descriptorSet_texture.clear();
	primFrom = nullptr;
	needUpdateTextures.clear();
	auto* manager = VulkanManager::GetManager();
	for (auto& i : descriptorSet_texture)
	{
		manager->FreeDescriptorSet(manager->GetDescriptorPool(), i);
	}
}

ModelPrimitive::~ModelPrimitive()
{
	transform = nullptr;
	vertexData.clear();
	vertexIndices.clear();
	renderer = nullptr;
}
