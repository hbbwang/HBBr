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
			Shader::_vsShader[prim->graphicsIndex.GetVSShaderName() + "@" + HString::FromUInt(prim->graphicsIndex.GetVSVarient())],
			Shader::_psShader[prim->graphicsIndex.GetPSShaderName() + "@" + HString::FromUInt(prim->graphicsIndex.GetPSVarient())]);
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
	prim->vertexData = prim->vertexInput.GetData(Shader::_vsShader[mat->graphicsIndex.GetVSShaderFullName()]->header.vertexInput);
	prim->vertexIndices = prim->vertexInput.vertexIndices;
	prim->vbSize = prim->vertexData.size() * sizeof(float);
	prim->ibSize = prim->vertexIndices.size() * sizeof(uint32_t);

	//顶点数据生成完毕，就把源数据卸载掉
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

float MaterialPrimitive::GetScalarParameter_VS(HString name, int* arrayIndex, int* vec4Index)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		return uniformBuffer_vs[it->arrayIndex][it->vec4Index];
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return 0;
}

glm::vec2 MaterialPrimitive::GetVector2Parameter_VS(HString name, int* arrayIndex, int* vec4Index)
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
		result.x = uniformBuffer_vs[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec3 MaterialPrimitive::GetVector3Parameter_VS(HString name, int* arrayIndex, int* vec4Index)
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
		result.x = uniformBuffer_vs[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1];
		result.z = uniformBuffer_vs[it->arrayIndex][it->vec4Index + 2];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec4 MaterialPrimitive::GetVector4Parameter_VS(HString name, int* arrayIndex, int* vec4Index)
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
		result.x = uniformBuffer_vs[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1];
		result.z = uniformBuffer_vs[it->arrayIndex][it->vec4Index + 2];
		result.w = uniformBuffer_vs[it->arrayIndex][it->vec4Index + 3];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

float MaterialPrimitive::GetScalarParameter_PS(HString name, int* arrayIndex, int* vec4Index)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		return uniformBuffer_ps[it->arrayIndex][it->vec4Index];
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return 0;
}

glm::vec2 MaterialPrimitive::GetVector2Parameter_PS(HString name, int* arrayIndex, int* vec4Index)
{
	glm::vec2 result = glm::vec2(0);
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = uniformBuffer_ps[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec3 MaterialPrimitive::GetVector3Parameter_PS(HString name, int* arrayIndex, int* vec4Index)
{
	glm::vec3 result = glm::vec3(0);
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = uniformBuffer_ps[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1];
		result.z = uniformBuffer_ps[it->arrayIndex][it->vec4Index + 2];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

glm::vec4 MaterialPrimitive::GetVector4Parameter_PS(HString name, int* arrayIndex, int* vec4Index)
{
	glm::vec4 result = glm::vec4(0);
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		if (arrayIndex != nullptr)		*arrayIndex = (int)it->arrayIndex;
		if (vec4Index != nullptr)		*vec4Index = (int)it->vec4Index;
		result.x = uniformBuffer_ps[it->arrayIndex][it->vec4Index];
		result.y = uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1];
		result.z = uniformBuffer_ps[it->arrayIndex][it->vec4Index + 2];
		result.w = uniformBuffer_ps[it->arrayIndex][it->vec4Index + 3];
		return result;
	}
	ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "A000026"), name.c_str());
	return result;
}

void MaterialPrimitive::SetScalarParameter_PS(HString name, float value)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		uniformBuffer_ps[it->arrayIndex][it->vec4Index] = value;
	}
}

void MaterialPrimitive::SetVec2Parameter_PS(HString name, glm::vec2 value)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		uniformBuffer_ps[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1] = value.y;
	}
}

void MaterialPrimitive::SetVec3Parameter_PS(HString name, glm::vec3 value)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		uniformBuffer_ps[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1] = value.y;
		uniformBuffer_ps[it->arrayIndex][it->vec4Index + 2] = value.z;
	}
}

void MaterialPrimitive::SetVec4Parameter_PS(HString name, glm::vec4 value)
{
	auto it = std::find_if(_paramterInfos_ps.begin(), _paramterInfos_ps.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_ps.end())
	{
		uniformBuffer_ps[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer_ps[it->arrayIndex][it->vec4Index + 1] = value.y;
		uniformBuffer_ps[it->arrayIndex][it->vec4Index + 2] = value.z;
		uniformBuffer_ps[it->arrayIndex][it->vec4Index + 3] = value.w;
	}
}

void MaterialPrimitive::SetScalarParameter_PS(int arrayIndex, int vec4Index, float value)
{
	uniformBuffer_ps[arrayIndex][vec4Index] = value;
}

void MaterialPrimitive::SetVec2Parameter_PS(int arrayIndex, int vec4Index, glm::vec2 value)
{
	uniformBuffer_ps[arrayIndex][vec4Index] = value.x;
	uniformBuffer_ps[arrayIndex][vec4Index + 1] = value.y;
}

void MaterialPrimitive::SetVec3Parameter_PS(int arrayIndex, int vec4Index, glm::vec3 value)
{
	uniformBuffer_ps[arrayIndex][vec4Index] = value.x;
	uniformBuffer_ps[arrayIndex][vec4Index + 1] = value.y;
	uniformBuffer_ps[arrayIndex][vec4Index + 2] = value.z;
}

void MaterialPrimitive::SetVec4Parameter_PS(int arrayIndex, int vec4Index, glm::vec4 value)
{
	uniformBuffer_ps[arrayIndex][vec4Index] = value.x;
	uniformBuffer_ps[arrayIndex][vec4Index + 1] = value.y;
	uniformBuffer_ps[arrayIndex][vec4Index + 2] = value.z;
	uniformBuffer_ps[arrayIndex][vec4Index + 3] = value.w;
}

void MaterialPrimitive::SetScalarParameter_VS(HString name, float value)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		uniformBuffer_vs[it->arrayIndex][it->vec4Index] = value;
	}
}

void MaterialPrimitive::SetVec2Parameter_VS(HString name, glm::vec2 value)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		uniformBuffer_vs[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1] = value.y;
	}
}

void MaterialPrimitive::SetVec3Parameter_VS(HString name, glm::vec3 value)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		uniformBuffer_vs[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1] = value.y;
		uniformBuffer_vs[it->arrayIndex][it->vec4Index + 2] = value.z;
	}
}

void MaterialPrimitive::SetVec4Parameter_VS(HString name, glm::vec4 value)
{
	auto it = std::find_if(_paramterInfos_vs.begin(), _paramterInfos_vs.end(),
		[name](MaterialParameterInfo& info) {
			return info.name == name;
		});
	if (it != _paramterInfos_vs.end())
	{
		uniformBuffer_vs[it->arrayIndex][it->vec4Index] = value.x;
		uniformBuffer_vs[it->arrayIndex][it->vec4Index + 1] = value.y;
		uniformBuffer_vs[it->arrayIndex][it->vec4Index + 2] = value.z;
		uniformBuffer_vs[it->arrayIndex][it->vec4Index + 3] = value.w;
	}
}

void MaterialPrimitive::SetScalarParameter_VS(int arrayIndex, int vec4Index, float value)
{
	uniformBuffer_vs[arrayIndex][vec4Index] = value;
}

void MaterialPrimitive::SetVec2Parameter_VS(int arrayIndex, int vec4Index, glm::vec2 value)
{
	uniformBuffer_vs[arrayIndex][vec4Index] = value.x;
	uniformBuffer_vs[arrayIndex][vec4Index + 1] = value.y;
}

void MaterialPrimitive::SetVec3Parameter_VS(int arrayIndex, int vec4Index, glm::vec3 value)
{
	uniformBuffer_vs[arrayIndex][vec4Index] = value.x;
	uniformBuffer_vs[arrayIndex][vec4Index + 1] = value.y;
	uniformBuffer_vs[arrayIndex][vec4Index + 2] = value.z;
}

void MaterialPrimitive::SetVec4Parameter_VS(int arrayIndex, int vec4Index, glm::vec4 value)
{
	uniformBuffer_vs[arrayIndex][vec4Index] = value.x;
	uniformBuffer_vs[arrayIndex][vec4Index + 1] = value.y;
	uniformBuffer_vs[arrayIndex][vec4Index + 2] = value.z;
	uniformBuffer_vs[arrayIndex][vec4Index + 3] = value.w;
}

void MaterialPrimitive::SetTextureSampler(int index, VkSampler sampler)
{
	_samplers[index] = sampler;
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
