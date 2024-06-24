#include "Primitive.h"
#include "Shader.h"

std::vector<std::vector<MaterialPrimitive*>> PrimitiveProxy::_allGraphicsPrimitives;

std::map<MaterialPrimitive*, std::map<class VulkanRenderer*, std::vector<ModelPrimitive*>>>  PrimitiveProxy::_allModelPrimitives;


void PrimitiveProxy::AddMaterialPrimitive(MaterialPrimitive* prim)
{
	if ((uint32_t)PrimitiveProxy::_allGraphicsPrimitives.size() != (uint32_t)Pass::MaxNum)
	{
		_allGraphicsPrimitives.resize((uint32_t)Pass::MaxNum);
	}
	_allGraphicsPrimitives[(uint32_t)prim->passUsing].push_back(prim);
}

void PrimitiveProxy::GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim, HString vsFullName, HString psFullName)
{
	if (vsFullName.Length() <= 1 || psFullName.Length() <= 1)
	{
		prim->graphicsIndex = PipelineIndex::GetPipelineIndex(
			Shader::_vsShader[prim->vsShader + "@" + HString::FromUInt(prim->graphicsIndex.GetVSVarient())],
			Shader::_psShader[prim->psShader + "@" + HString::FromUInt(prim->graphicsIndex.GetPSVarient())]);
	}
	else
	{
		prim->graphicsIndex = PipelineIndex::GetPipelineIndex(
			Shader::_vsShader[vsFullName],
			Shader::_psShader[psFullName]);
	}
}

void PrimitiveProxy::GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim, std::weak_ptr<ShaderCache> vs, std::weak_ptr<ShaderCache> ps)
{
	prim->graphicsIndex = PipelineIndex::GetPipelineIndex(
		vs,
		ps);
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
		_allGraphicsPrimitives[index].erase(it);
	}
}

void PrimitiveProxy::AddModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim, class VulkanRenderer* renderer)
{
	prim->vertexData = prim->vertexInput.GetData(Shader::_vsShader[mat->vsShader + "@" + HString::FromUInt(mat->graphicsIndex.GetVSVarient())]->header.vertexInput);
	prim->vertexIndices = prim->vertexInput.vertexIndices;
	prim->vbSize = prim->vertexData.size() * sizeof(float);
	prim->ibSize = prim->vertexIndices.size() * sizeof(uint32_t);
	//
	prim->vertexInput = VertexFactory::VertexInput();
	auto it = _allModelPrimitives[mat].find(renderer);
	if (it == _allModelPrimitives[mat].end())
	{
		_allModelPrimitives[mat].emplace(renderer, std::vector<ModelPrimitive*>());
	}
	_allModelPrimitives[mat][renderer].push_back(prim);
}

void PrimitiveProxy::RemoveModelPrimitive(MaterialPrimitive* mat, ModelPrimitive* prim, class VulkanRenderer* renderer)
{
	if(prim)
	{
		auto it = _allModelPrimitives.find(mat);
		if (it != _allModelPrimitives.end())
		{
			auto rit = it->second.find(renderer);
			if (rit != it->second.end())
			{
				auto pit = std::find(rit->second.begin(), rit->second.end(), prim);
				if (pit != rit->second.end())
				{
					rit->second.erase(pit);
				}
			}
		}
	}
}

void MaterialPrimitive::SetTexture(int index, Texture2D* newTexture)
{
	textures[index] = newTexture;
	_needUpdateDescriptorSet_tex = true;
}

void MaterialPrimitive::SetTexture(HString textureName, Texture2D* newTexture)
{
	auto it = std::find_if(_textureInfos.begin(), _textureInfos.end(), [textureName](MaterialTextureInfo& info) {
		return info.name.IsSame(textureName, false);
	});
	if (it != _textureInfos.end())
	{
		SetTexture(it->index, newTexture);
	}
}

float MaterialPrimitive::GetScalarParameter(HString name, int* arrayIndex, int* vec4Index)
{
	auto it = std::find_if(_paramterInfos.begin(), _paramterInfos.end(), 
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		return uniformBuffer[it->arrayIndex][it->vec4Index];
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return 0;
}

glm::vec2 MaterialPrimitive::GetVector2Parameter(HString name, int* arrayIndex, int* vec4Index)
{
	glm::vec2 result = glm::vec2(0);
	auto it = std::find_if(_paramterInfos.begin(), _paramterInfos.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = uniformBuffer[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer[it->arrayIndex][it->vec4Index + 1];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec3 MaterialPrimitive::GetVector3Parameter(HString name, int* arrayIndex, int* vec4Index)
{
	glm::vec3 result = glm::vec3(0);
	auto it = std::find_if(_paramterInfos.begin(), _paramterInfos.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = uniformBuffer[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer[it->arrayIndex][it->vec4Index + 1];
		result.z = uniformBuffer[it->arrayIndex][it->vec4Index + 2];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec4 MaterialPrimitive::GetVector4Parameter(HString name, int* arrayIndex, int* vec4Index)
{
	glm::vec4 result = glm::vec4(0);
	auto it = std::find_if(_paramterInfos.begin(), _paramterInfos.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = uniformBuffer[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer[it->arrayIndex][it->vec4Index + 1];
		result.z = uniformBuffer[it->arrayIndex][it->vec4Index + 2];
		result.w = uniformBuffer[it->arrayIndex][it->vec4Index + 3];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

void MaterialPrimitive::SetScalarParameter(HString name, float value)
{
	auto it = std::find_if(_paramterInfos.begin(), _paramterInfos.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos.end())
	{
		uniformBuffer[it->arrayIndex][it->vec4Index] = value;
	}
}

void MaterialPrimitive::SetVec2Parameter(HString name, glm::vec2 value)
{
	auto it = std::find_if(_paramterInfos.begin(), _paramterInfos.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos.end())
	{
		uniformBuffer[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer[it->arrayIndex][it->vec4Index + 1] = value.y;
	}
}

void MaterialPrimitive::SetVec3Parameter(HString name, glm::vec3 value)
{
	auto it = std::find_if(_paramterInfos.begin(), _paramterInfos.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos.end())
	{
		uniformBuffer[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer[it->arrayIndex][it->vec4Index + 1] = value.y;
		uniformBuffer[it->arrayIndex][it->vec4Index + 2] = value.z;
	}
}

void MaterialPrimitive::SetVec4Parameter(HString name, glm::vec4 value)
{
	auto it = std::find_if(_paramterInfos.begin(), _paramterInfos.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos.end())
	{
		uniformBuffer[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer[it->arrayIndex][it->vec4Index + 1] = value.y;
		uniformBuffer[it->arrayIndex][it->vec4Index + 2] = value.z;
		uniformBuffer[it->arrayIndex][it->vec4Index + 3] = value.w;
	}
}

void MaterialPrimitive::SetScalarParameter(int arrayIndex, int vec4Index, float value)
{
	uniformBuffer[arrayIndex][vec4Index] = value;
}

void MaterialPrimitive::SetVec2Parameter(int arrayIndex, int vec4Index, glm::vec2 value)
{
	uniformBuffer[arrayIndex][vec4Index] = value.x;
	uniformBuffer[arrayIndex][vec4Index + 1] = value.y;
}

void MaterialPrimitive::SetVec3Parameter(int arrayIndex, int vec4Index, glm::vec3 value)
{
	uniformBuffer[arrayIndex][vec4Index] = value.x;
	uniformBuffer[arrayIndex][vec4Index + 1] = value.y;
	uniformBuffer[arrayIndex][vec4Index + 2] = value.z;
}

void MaterialPrimitive::SetVec4Parameter(int arrayIndex, int vec4Index, glm::vec4 value)
{
	uniformBuffer[arrayIndex][vec4Index] = value.x;
	uniformBuffer[arrayIndex][vec4Index + 1] = value.y;
	uniformBuffer[arrayIndex][vec4Index + 2] = value.z;
	uniformBuffer[arrayIndex][vec4Index + 3] = value.w;
}

void MaterialPrimitive::SetTextureSampler(int index, VkSampler sampler)
{
	_samplers[index] = sampler;
	_needUpdateDescriptorSet_tex = true;
}

void MaterialPrimitive::SetTextureSampler(HString textureName, VkSampler sampler)
{
	auto it = std::find_if(_textureInfos.begin(), _textureInfos.end(), [textureName](MaterialTextureInfo& info) {
		return info.name.IsSame(textureName, false);
		});
	if (it != _textureInfos.end())
	{
		SetTextureSampler(it->index, sampler);
	}
}
