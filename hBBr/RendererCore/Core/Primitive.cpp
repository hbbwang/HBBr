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
			&Shader::_vsShader[prim->vsShader + "@" + HString::FromUInt(prim->graphicsIndex.GetVSVarient())],
			&Shader::_psShader[prim->psShader + "@" + HString::FromUInt(prim->graphicsIndex.GetPSVarient())]);
	}
	else
	{
		prim->graphicsIndex = PipelineIndex::GetPipelineIndex(
			&Shader::_vsShader[vsFullName],
			&Shader::_psShader[psFullName]);
	}
}

void PrimitiveProxy::GetNewMaterialPrimitiveIndex(MaterialPrimitive* prim, ShaderCache& vs, ShaderCache& ps)
{
	prim->graphicsIndex = PipelineIndex::GetPipelineIndex(
		&vs,
		&ps);
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
	prim->vertexData = prim->vertexInput.GetData(Shader::_vsShader[mat->vsShader + "@" + HString::FromUInt(mat->graphicsIndex.GetVSVarient())].header.vertexInput);
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
