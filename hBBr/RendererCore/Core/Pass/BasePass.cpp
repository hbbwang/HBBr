﻿#include "BasePass.h"
#include "VulkanRenderer.h"
#include "DescriptorSet.h"
#include "VertexFactory.h"
#include "Buffer.h"
#include "Primitive.h"
#include "Pass/PassType.h"
#include "Texture2D.h"
#include "ConsoleDebug.h"
#include "PassManager.h"
/*
	Opaque pass
*/
#pragma region BasePass
BasePass::~BasePass()
{
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_m_t);
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_m);
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_t);
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o);
}

void BasePass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//SceneDepth
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture((uint32_t)SceneTextureDesc::SceneDepth)->GetFormat() , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	AddSubpass({}, { 0 }, 1);
	CreateRenderPass();
	//Texture2D DescriptorSet
	auto manager = VulkanManager::GetManager();
	_opaque_descriptorSet_pass.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, sizeof(PassUniformBuffer)));
	_opaque_descriptorSet_obj.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_opaque_descriptorSet_mat.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1));
	_opaque_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	_opaque_indexBuffer.reset(new Buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	manager->CreatePipelineLayout(
		{
			_opaque_descriptorSet_pass->GetDescriptorSetLayout(),
			_opaque_descriptorSet_obj->GetDescriptorSetLayout(),
			_opaque_descriptorSet_mat->GetDescriptorSetLayout(),
			manager->GetImageDescriptorSetLayout()
		}
	, _pipelineLayout_p_o_m_t);
	manager->CreatePipelineLayout(
		{
			_opaque_descriptorSet_pass->GetDescriptorSetLayout(),
			_opaque_descriptorSet_obj->GetDescriptorSetLayout(),
			_opaque_descriptorSet_mat->GetDescriptorSetLayout(),
		}
	, _pipelineLayout_p_o_m);
	manager->CreatePipelineLayout(
		{
			_opaque_descriptorSet_pass->GetDescriptorSetLayout(),
			_opaque_descriptorSet_obj->GetDescriptorSetLayout(),
		}
	, _pipelineLayout_p_o);
	manager->CreatePipelineLayout(
		{
			_opaque_descriptorSet_pass->GetDescriptorSetLayout(),
			_opaque_descriptorSet_obj->GetDescriptorSetLayout(),
			manager->GetImageDescriptorSetLayout()
		}
	, _pipelineLayout_p_o_t);
	//Pass Uniform总是一尘不变的,并且我们用的是Dynamic uniform buffer ,所以只需要更新一次所有的DescriptorSet即可。
	_opaque_descriptorSet_pass->UpdateDescriptorSetAll(sizeof(PassUniformBuffer));
	_passName = "Opaque Render Pass";
}

void BasePass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), _markColor);
	//Update FrameBuffer

	auto finalColor = GetSceneTexture(SceneTextureDesc::FinalColor);
	auto depth = GetSceneTexture(SceneTextureDesc::SceneDepth);
	ResetFrameBufferCustom(_renderer->GetRenderSize(), { finalColor->GetTextureView(), depth->GetTextureView() });
	SetViewport(_currentFrameBufferSize);
	BeginRenderPass({ 0,0,0,0 });
	//Opaque Pass
	SetupBasePassAndDraw(
		Pass::OpaquePass,
		_opaque_descriptorSet_pass.get(),
		_opaque_descriptorSet_obj.get(),
		_opaque_descriptorSet_mat.get(),
		_opaque_vertexBuffer.get(),
		_opaque_indexBuffer.get()
	);
	
	//manager->CmdNextSubpass(cmdBuf);
	//Translucent pass
	//...

	EndRenderPass();
}

void BasePass::PassReset()
{

}

void BasePass::SetupBasePassAndDraw(Pass p, DescriptorSet* pass, DescriptorSet* obj, DescriptorSet* mat, Buffer* vb, Buffer* ib)
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	VkDeviceSize vbOffset = 0;
	VkDeviceSize ibOffset = 0;
	VkDeviceSize matOffset = 0;
	uint32_t objectUboOffset = 0;
	std::vector<uint32_t> matBufferOffset;
	std::vector<uint32_t> matBufferSize;
	uint32_t textureDescriptorCount = 0;
	pipelineTemps.clear();
	auto frameIndex = _renderer->GetCurrentFrameIndex();
	VkDescriptorSet tds = VK_NULL_HANDLE;
	//Reset buffer
	{
		auto matPrim = PrimitiveProxy::GetMaterialPrimitives((uint32_t)p);
		if (!matPrim)//Can not find any materials,return.
		{
			return;
		}
		bool bUpdateObjUb = false;
		//Update pass uniform buffers
		{
			PassUniformBuffer passUniformBuffer = _manager->GetPassUniformBufferCache();
			pass->BufferMapping(&passUniformBuffer, 0, sizeof(PassUniformBuffer));
		}
		uint32_t objectCount = 0;
		for (auto m : *matPrim)
		{
			//Get Pipeline
 			auto pipelineObj = PipelineManager::GetGraphicsPipelineMap(m->graphicsIndex);
			if (pipelineObj == nullptr)
			{
				HString vsShaderFullName = m->graphicsIndex.GetVSShaderFullName();
				HString psShaderFullName = m->graphicsIndex.GetPSShaderFullName();
				//auto vsCacheIt = Shader::_vsShader.find(vsShaderFullName);
				//auto psCacheIt = Shader::_psShader.find(psShaderFullName);

				auto vsCache = Shader::_vsShader[vsShaderFullName];
				auto psCache = Shader::_psShader[psShaderFullName];

				VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
				PipelineManager::SetColorBlend(pipelineCreateInfo, false);
				PipelineManager::SetColorBlend(pipelineCreateInfo, false);
				PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
				PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
				PipelineManager::SetVertexInput(pipelineCreateInfo, m->inputLayout);
				PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache.get(), psCache.get());
				//Setting pipeline end
				VkPipelineLayout pipelineLayout;
				if (pipelineCreateInfo.bHasMaterialTexture)
				{
					if (pipelineCreateInfo.bHasMaterialParameter)
						pipelineLayout = _pipelineLayout_p_o_m_t;
					else
						pipelineLayout = _pipelineLayout_p_o_t;
				}
				else
				{
					if (pipelineCreateInfo.bHasMaterialParameter)
						pipelineLayout = _pipelineLayout_p_o_m;
					else
						pipelineLayout = _pipelineLayout_p_o;
				}
				pipelineObj = PipelineManager::CreatePipelineObject(pipelineCreateInfo, pipelineLayout,
					_renderPass, m->graphicsIndex, (uint32_t)_subpassDescs.size());
			}

			if (pipelineTemps.capacity() < pipelineTemps.size() + 1)
				pipelineTemps.reserve(( pipelineTemps.size() + 1 ) * 2);
			pipelineTemps.push_back(pipelineObj);

			if (m->uniformBufferSize != 0)
			{
				////Test
				//m->uniformBuffer[2] = glm::vec4(1,0,0,1);
				mat->BufferMapping(m->uniformBuffer.data(), 0, m->uniformBufferSize);
				matBufferOffset.push_back((uint32_t)matOffset);
				matBufferSize.push_back((uint32_t)m->uniformBufferSize);
				matOffset += m->uniformBufferSize;
			}


			if (m->GetTextures().size() > 0)
			{
				auto tex_it = _descriptorSet_tex.find(m);
				if (tex_it == _descriptorSet_tex.end())
				{
					std::vector<TextureDescriptorSet> new_tds;
					if (new_tds.size() != manager->GetSwapchainBufferCount())
					{
						new_tds.resize(manager->GetSwapchainBufferCount());
						for (auto& i : new_tds)
						{
							i.texCache = m->GetTextures();
							manager->AllocateDescriptorSet(manager->GetDescriptorPool(), manager->GetImageDescriptorSetLayout(), i.descriptorSet_tex);
							manager->UpdateTextureDescriptorSet(i.descriptorSet_tex, m->GetTextures(), m->GetSamplers());
						}
					}	
					auto pair = _descriptorSet_tex.emplace(m, new_tds);
					tds = pair.first->second[frameIndex].descriptorSet_tex;
				}
				else
				{
					auto& tds_obj = tex_it->second[frameIndex];
					if (tds_obj.texCache != m->GetTextures())
					{
						tds_obj.texCache = m->GetTextures();
						manager->UpdateTextureDescriptorSet(tds_obj.descriptorSet_tex, m->GetTextures(), m->GetSamplers());
					}
					tds = tds_obj.descriptorSet_tex;
				}
				
			}

			auto prims = PrimitiveProxy::GetModelPrimitives(m, _renderer);
			if (prims)
			{
				for (size_t primIndex = 0; primIndex < prims->size(); primIndex++)
				{
					ModelPrimitive* prim = prims->at(primIndex);
					bool bRefreshBuffer = prim->bNeedUpdate;
					{
						//Vb,Ib
						if (vbOffset != prim->vbPos || bRefreshBuffer) //偏移信息不相同时,重新映射
						{
							if (prim->bActive)
							{
								vb->BufferMapping(prim->vertexData.data(), vbOffset, prim->vbSize);
								prim->vbPos = vbOffset;
							}
							bRefreshBuffer = true;
						}
						if (ibOffset != prim->ibPos || bRefreshBuffer) //偏移信息不相同时,重新映射
						{
							if (prim->bActive)
							{
								ib->BufferMapping(prim->vertexIndices.data(), ibOffset, prim->ibSize);
								prim->ibPos = ibOffset;
							}
							bRefreshBuffer = true;
						}
						if (prim->bActive)
						{
							vbOffset += prim->vbSize;
							ibOffset += prim->ibSize;
						}
						//Object uniform
						if (prim->transform &&
							(prim->transform->NeedUpdateUb()
								|| bRefreshBuffer //当顶点发生改变的时候,UniformBuffer也应该一起更新。
								))
						{
							if (prim->bActive)
							{
								ObjectUniformBuffer objectUniformBuffer = {};
								objectUniformBuffer.WorldMatrix = prim->transform->GetWorldMatrix();
								obj->BufferMapping(&objectUniformBuffer, (uint64_t)objectUboOffset, sizeof(ObjectUniformBuffer));
							}
							bUpdateObjUb = true;
						}
						if (prim->bActive)
						{
							objectCount++;
							objectUboOffset += (uint32_t)sizeof(ObjectUniformBuffer);
						}
					}
				}
			}
		}
		if (bUpdateObjUb)
		{
			obj->ResizeDescriptorBuffer(sizeof(ObjectUniformBuffer) * (objectCount + 1));
		}
		obj->UpdateDescriptorSet(sizeof(ObjectUniformBuffer));
		mat->ResizeDescriptorBuffer(matOffset);
		mat->UpdateDescriptorSet(matBufferSize, matBufferOffset);
	}

	//Update Render
	vbOffset = 0;
	ibOffset = 0;
	objectUboOffset = 0;
	uint32_t dynamicOffset[] = { 0 };
	uint32_t matIndex = 0;
	auto matPrims = PrimitiveProxy::GetMaterialPrimitives((uint32_t)p);
	for (auto m : *matPrims)
	{
		auto curPipeline = pipelineTemps[matIndex];
		//Pass uniform buffers
		{
			vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 0, 1, &pass->GetDescriptorSet(), 1, dynamicOffset);
		}
		manager->CmdCmdBindPipeline(cmdBuf, curPipeline->pipeline);
		//Material uniform buffers
		if (curPipeline->bHasMaterialParameter)
		{
			vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &mat->GetDescriptorSet(), 1, &matBufferOffset[matIndex]);
		}
		//Material texture buffers
		if (curPipeline->bHasMaterialTexture)
		{
			vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &tds, 0, nullptr);
		}
		//Objects
		auto prims = PrimitiveProxy::GetModelPrimitives(m, _renderer);
		if (prims)
		{
			for (size_t pi = 0; pi < prims->size(); pi++)
			{
				ModelPrimitive* prim = prims->at(pi);
				if (prim->renderer == this->_renderer &&  prim->bActive)
				{
					vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 1, 1, &obj->GetDescriptorSet(), 1, &objectUboOffset);
					objectUboOffset += sizeof(ObjectUniformBuffer);

					VkBuffer verBuf[] = { vb->GetBuffer() };
					vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);
					vkCmdBindIndexBuffer(cmdBuf, ib->GetBuffer(), ibOffset, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(cmdBuf, (uint32_t)prim->vertexIndices.size(), 1, 0, 0, 0);

					vbOffset += prim->vbSize;
					ibOffset += prim->ibSize;
				}
			}
		}	
		matIndex++;
	}
}

#pragma endregion BasePass