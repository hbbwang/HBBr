#include "BasePass.h"
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
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_vsm);
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_psm);
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_vspsm);
	VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o);
	_pipelineLayout_p_o_vsm = VK_NULL_HANDLE;
	_pipelineLayout_p_o_psm = VK_NULL_HANDLE;
	_pipelineLayout_p_o_vspsm = VK_NULL_HANDLE;
	_pipelineLayout_p_o = VK_NULL_HANDLE;
	for (int i = 0; i < _maxTextureBinding; i++)
	{
		VulkanManager::GetManager()->DestroyDescriptorSetLayout(_descriptorSetLayout_tex[i]);
		VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_vsm_t[i]);
		VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_psm_t[i]);
		VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_vspsm_t[i]);
		VulkanManager::GetManager()->DestroyPipelineLayout(_pipelineLayout_p_o_t[i]);
		_descriptorSetLayout_tex[i] = VK_NULL_HANDLE;
		_pipelineLayout_p_o_vsm_t[i] = VK_NULL_HANDLE;
		_pipelineLayout_p_o_psm_t[i] = VK_NULL_HANDLE;
		_pipelineLayout_p_o_vspsm_t[i] = VK_NULL_HANDLE;
		_pipelineLayout_p_o_t[i] = VK_NULL_HANDLE;
	}
	_descriptorSetLayout_tex.clear();
	_pipelineLayout_p_o_vsm_t.clear();
	_pipelineLayout_p_o_psm_t.clear();
	_pipelineLayout_p_o_vspsm_t.clear();
	_pipelineLayout_p_o_t.clear();

}

void BasePass::PassInit()
{
	//一个shader，最高可以绑定16张纹理和采样器
	_maxTextureBinding = 16;
	_descriptorSetLayout_tex.resize(16);
	_pipelineLayout_p_o_vsm_t.resize(16);
	_pipelineLayout_p_o_psm_t.resize(16);
	_pipelineLayout_p_o_vspsm_t.resize(16);
	_pipelineLayout_p_o_t.resize(16);

	//SceneDepth	: 0
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture(SceneTextureDesc::SceneDepth)->GetFormat(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	//SceneColor	: 1
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture(SceneTextureDesc::SceneColor)->GetFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//GBuffer0		: 2
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture(SceneTextureDesc::GBuffer0)->GetFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//GBuffer1		: 3
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture(SceneTextureDesc::GBuffer1)->GetFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//GBuffer2		: 4
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, GetSceneTexture(SceneTextureDesc::GBuffer2)->GetFormat(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//改了ColorAttachment的话，记得改一下下面的 blendState.outputAttachmentCount 数量
	AddSubpass({}, { 1,2,3,4 }, 0);
	CreateRenderPass();

	auto manager = VulkanManager::GetManager();
	//Texture2D DescriptorSet
	_opaque_descriptorSet_pass.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, PipelineManager::GetUniformBufferDynamicLayoutVSPS(), sizeof(PassUniformBuffer)));
	_opaque_descriptorSet_obj.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, PipelineManager::GetUniformBufferDynamicLayoutVSPS(), BufferSizeRange));
	_opaque_descriptorSet_mat_vs.reset(new DescriptorSet(_renderer, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC }, PipelineManager::GetUniformBufferDynamicLayoutVS(), BufferSizeRange, { VK_SHADER_STAGE_VERTEX_BIT }));
	_opaque_descriptorSet_mat_ps.reset(new DescriptorSet(_renderer, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC }, PipelineManager::GetUniformBufferDynamicLayoutPS(), BufferSizeRange, { VK_SHADER_STAGE_FRAGMENT_BIT }));
	_opaque_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	_opaque_indexBuffer.reset(new Buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT));

	for (int i = 0; i < _maxTextureBinding; i++)
	{
		manager->CreateDescripotrSetLayout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i + 1, _descriptorSetLayout_tex[i], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		manager->CreatePipelineLayout(
			{
				PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
				PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
				PipelineManager::GetUniformBufferDynamicLayoutVS(), 
				_descriptorSetLayout_tex[i]
			}
		, _pipelineLayout_p_o_vsm_t[i]); 
		manager->CreatePipelineLayout(
			{
				PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
				PipelineManager::GetUniformBufferDynamicLayoutVSPS(), 
				 PipelineManager::GetUniformBufferDynamicLayoutPS(), 
				_descriptorSetLayout_tex[i] 
			}
		, _pipelineLayout_p_o_psm_t[i]); 
		manager->CreatePipelineLayout(
			{
				PipelineManager::GetUniformBufferDynamicLayoutVSPS(), 
				PipelineManager::GetUniformBufferDynamicLayoutVSPS(), 
				PipelineManager::GetUniformBufferDynamicLayoutVS(),
				 PipelineManager::GetUniformBufferDynamicLayoutPS(),
				_descriptorSetLayout_tex[i]
			}
		, _pipelineLayout_p_o_vspsm_t[i]);
		manager->CreatePipelineLayout(
			{
				PipelineManager::GetUniformBufferDynamicLayoutVSPS(), 
				PipelineManager::GetUniformBufferDynamicLayoutVSPS(), 
				_descriptorSetLayout_tex[i] 
			}
		, _pipelineLayout_p_o_t[i]); 
	}

	manager->CreatePipelineLayout(
		{
			PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
			PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
			PipelineManager::GetUniformBufferDynamicLayoutVS(),
		}
	, _pipelineLayout_p_o_vsm);
	manager->CreatePipelineLayout(
		{
			PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
			PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
			 PipelineManager::GetUniformBufferDynamicLayoutPS(),
		}
	, _pipelineLayout_p_o_psm);
	manager->CreatePipelineLayout(
		{
			PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
			PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
			PipelineManager::GetUniformBufferDynamicLayoutVS(),
			 PipelineManager::GetUniformBufferDynamicLayoutPS(),
		}
	, _pipelineLayout_p_o_vspsm);
	manager->CreatePipelineLayout(
		{
			PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
			PipelineManager::GetUniformBufferDynamicLayoutVSPS(),
		}
	, _pipelineLayout_p_o);

	//Pass Uniform总是一尘不变的,并且我们用的是Dynamic uniform buffer ,所以只需要更新一次所有的DescriptorSet即可。
	_opaque_descriptorSet_pass->UpdateDescriptorSetAll(sizeof(PassUniformBuffer));
}

void BasePass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.15, 0.16, 0.18, 0.8));
	//Transition
	auto sceneDepth = GetSceneTexture(SceneTextureDesc::SceneDepth);
	auto sceneColor = GetSceneTexture(SceneTextureDesc::SceneColor);
	auto gbuffer0 = GetSceneTexture(SceneTextureDesc::GBuffer0);
	auto gbuffer1 = GetSceneTexture(SceneTextureDesc::GBuffer1);
	auto gbuffer2 = GetSceneTexture(SceneTextureDesc::GBuffer2);
	if (gbuffer0->GetLayout() == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		sceneDepth->Transition(cmdBuf,	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		sceneColor->Transition(cmdBuf,	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		gbuffer0->Transition(cmdBuf,	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		gbuffer1->Transition(cmdBuf,	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		gbuffer2->Transition(cmdBuf,	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	//Update FrameBuffer
	ResetFrameBufferCustom(_renderer->GetRenderSize(), 
		{ 
			GetSceneTexture(SceneTextureDesc::SceneDepth)->GetTextureView(),
			GetSceneTexture(SceneTextureDesc::SceneColor)->GetTextureView(),
			GetSceneTexture(SceneTextureDesc::GBuffer0)->GetTextureView(),
			GetSceneTexture(SceneTextureDesc::GBuffer1)->GetTextureView(),
			GetSceneTexture(SceneTextureDesc::GBuffer2)->GetTextureView(),
		});
	SetViewport(_currentFrameBufferSize);
	BeginRenderPass({ 0,0,0,0 });

	//Opaque Base Pass
	SetupPassAndDraw(Pass::OpaquePass);
	
	//manager->CmdNextSubpass(cmdBuf);
	//Translucent pass
	//...

	EndRenderPass();

	//输出完毕，转换到VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 方便后续Shader读取
	if (gbuffer0->GetLayout() == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		sceneDepth->Transition(cmdBuf,	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		gbuffer0->Transition(cmdBuf,	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		gbuffer1->Transition(cmdBuf,	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		gbuffer2->Transition(cmdBuf,	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}

void BasePass::PassReset()
{

}

void BasePass::SetupPassAndDraw(Pass p)
{
	auto& pass = _opaque_descriptorSet_pass;
	auto& obj = _opaque_descriptorSet_obj;
	auto& mat_vs = _opaque_descriptorSet_mat_vs;
	auto& mat_ps = _opaque_descriptorSet_mat_ps;
	auto &vb = _opaque_vertexBuffer;
	auto &ib = _opaque_indexBuffer;

	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	VkDeviceSize vbOffset = 0;
	VkDeviceSize ibOffset = 0;
	VkDeviceSize matOffset_vs = 0;
	VkDeviceSize matOffset_ps = 0;
	std::vector<uint32_t> matBufferOffset_vs;
	std::vector<uint32_t> matBufferSize_vs;
	std::vector<uint32_t> matBufferOffset_ps;
	std::vector<uint32_t> matBufferSize_ps;
	uint32_t objectUboOffset = 0;
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
				//清空纹理描述符集
				auto m_dst_it = _descriptorSet_tex.find(m);
				if (m_dst_it != _descriptorSet_tex.end())
				{
					_descriptorSet_tex.erase(m_dst_it);
				}

				HString vsShaderFullName = m->graphicsIndex.GetVSShaderFullName();
				HString psShaderFullName = m->graphicsIndex.GetPSShaderFullName();
				//auto vsCacheIt = Shader::_vsShader.find(vsShaderFullName);
				//auto psCacheIt = Shader::_psShader.find(psShaderFullName);

				auto vsCache = Shader::_vsShader[vsShaderFullName];
				auto psCache = Shader::_psShader[psShaderFullName];

				VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
				StaticBlendState blendState = {};
				blendState.outputAttachmentCount = psCache->header.colorAttachmentCount;
				PipelineManager::SetColorBlend(pipelineCreateInfo, false, blendState);
				PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
				PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
				PipelineManager::SetVertexInput(pipelineCreateInfo, m->inputLayout);
				PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache.get(), psCache.get());
				//Setting pipeline end
				VkPipelineLayout pipelineLayout;
				if (pipelineCreateInfo.bHasMaterialTexture)
				{
					if (pipelineCreateInfo.bHasMaterialParameterVS && !pipelineCreateInfo.bHasMaterialParameterPS)
						pipelineLayout = _pipelineLayout_p_o_vsm_t[psCache->header.shaderTextureCount];
					else if (pipelineCreateInfo.bHasMaterialParameterPS && !pipelineCreateInfo.bHasMaterialParameterVS)
						pipelineLayout = _pipelineLayout_p_o_psm_t[psCache->header.shaderTextureCount];
					else if (pipelineCreateInfo.bHasMaterialParameterPS && pipelineCreateInfo.bHasMaterialParameterVS)
						pipelineLayout = _pipelineLayout_p_o_vspsm_t[psCache->header.shaderTextureCount];
					else
						pipelineLayout = _pipelineLayout_p_o_t[psCache->header.shaderTextureCount];
				}
				else
				{
					if (pipelineCreateInfo.bHasMaterialParameterVS && !pipelineCreateInfo.bHasMaterialParameterPS)
						pipelineLayout = _pipelineLayout_p_o_vsm;
					else if (pipelineCreateInfo.bHasMaterialParameterPS && !pipelineCreateInfo.bHasMaterialParameterVS)
						pipelineLayout = _pipelineLayout_p_o_psm;
					else if (pipelineCreateInfo.bHasMaterialParameterPS && pipelineCreateInfo.bHasMaterialParameterVS)
						pipelineLayout = _pipelineLayout_p_o_vspsm;
					else
						pipelineLayout = _pipelineLayout_p_o;
				}
				pipelineObj = PipelineManager::CreatePipelineObject(pipelineCreateInfo, pipelineLayout,
					_renderPass, m->graphicsIndex, (uint32_t)_subpassDescs.size());
			}

			if (pipelineTemps.capacity() < pipelineTemps.size() + 1)
				pipelineTemps.reserve(( pipelineTemps.size() + 1 ) * 2);
			pipelineTemps.push_back(pipelineObj);

			if (m->uniformBufferSize_vs != 0)
			{
				mat_vs->BufferMapping(m->uniformBuffer_vs.data(), 0, m->uniformBufferSize_vs);
				matBufferOffset_vs.push_back((uint32_t)matOffset_vs);
				matBufferSize_vs.push_back((uint32_t)m->uniformBufferSize_vs);
				matOffset_vs += m->uniformBufferSize_vs;
			}
			if (m->uniformBufferSize_ps != 0)
			{
				mat_ps->BufferMapping(m->uniformBuffer_ps.data(), 0, m->uniformBufferSize_ps);
				matBufferOffset_ps.push_back((uint32_t)matOffset_ps);
				matBufferSize_ps.push_back((uint32_t)m->uniformBufferSize_ps);
				matOffset_ps += m->uniformBufferSize_ps;
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
							manager->AllocateDescriptorSet(manager->GetDescriptorPool(), _descriptorSetLayout_tex[m->GetTextures().size()], i.descriptorSet_tex);
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
		mat_vs->ResizeDescriptorBuffer(matOffset_vs);
		mat_vs->UpdateDescriptorSet(matBufferSize_vs, matBufferOffset_vs);
		mat_ps->ResizeDescriptorBuffer(matOffset_ps);
		mat_ps->UpdateDescriptorSet(matBufferSize_ps, matBufferOffset_ps);
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
		//Objects
		auto prims = PrimitiveProxy::GetModelPrimitives(m, _renderer);
		const uint32_t numPrims = prims->size();
		if (prims && numPrims > 0)
		{
			auto curPipeline = pipelineTemps[matIndex];
			//Pass uniform buffers
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 0, 1, &pass->GetDescriptorSet(), 1, dynamicOffset);
			}
			manager->CmdCmdBindPipeline(cmdBuf, curPipeline->pipeline, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS);
			//Material uniform buffers & Textures
			if (curPipeline->bHasMaterialParameterVS && curPipeline->bHasMaterialParameterPS && curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &mat_vs->GetDescriptorSet(), 1, &matBufferOffset_vs[matIndex]);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &mat_ps->GetDescriptorSet(), 1, &matBufferOffset_ps[matIndex]);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 4, 1, &tds, 0, nullptr);
			}
			else if (curPipeline->bHasMaterialParameterVS && curPipeline->bHasMaterialParameterPS && !curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &mat_vs->GetDescriptorSet(), 1, &matBufferOffset_vs[matIndex]);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &mat_ps->GetDescriptorSet(), 1, &matBufferOffset_ps[matIndex]);
			}
			else if (curPipeline->bHasMaterialParameterVS && !curPipeline->bHasMaterialParameterPS && !curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &mat_vs->GetDescriptorSet(), 1, &matBufferOffset_vs[matIndex]);
			}
			else if (!curPipeline->bHasMaterialParameterVS && curPipeline->bHasMaterialParameterPS && !curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &mat_ps->GetDescriptorSet(), 1, &matBufferOffset_ps[matIndex]);
			}
			else if (!curPipeline->bHasMaterialParameterVS && curPipeline->bHasMaterialParameterPS && curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &mat_ps->GetDescriptorSet(), 1, &matBufferOffset_ps[matIndex]);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &tds, 0, nullptr);
			}
			else if (curPipeline->bHasMaterialParameterVS && !curPipeline->bHasMaterialParameterPS && curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &mat_vs->GetDescriptorSet(), 1, &matBufferOffset_ps[matIndex]);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &tds, 0, nullptr);
			}

			for (size_t pi = 0; pi < prims->size(); pi++)
			{
				ModelPrimitive* prim = prims->at(pi);
				if (prim->renderer == this->_renderer && prim->bActive)
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
