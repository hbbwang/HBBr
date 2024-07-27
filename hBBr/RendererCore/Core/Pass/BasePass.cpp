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
}

void BasePass::PassInit()
{

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
	_opaque_descriptorSet_pass.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, PipelineManager::GetDescriptorSetLayout_UniformBufferDynamicVSPS(), sizeof(PassUniformBuffer)));
	_opaque_descriptorSet_obj.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, PipelineManager::GetDescriptorSetLayout_UniformBufferDynamicVSPS(), 32));
	_opaque_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 32));
	_opaque_indexBuffer.reset(new Buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 32));

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
	auto &vb = _opaque_vertexBuffer;
	auto &ib = _opaque_indexBuffer;
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	uint32_t matOffset_vs = 0;
	uint32_t matOffset_ps = 0;
	std::vector<uint32_t> matBufferOffset_vs;
	std::vector<uint32_t> matBufferSize_vs;
	std::vector<uint32_t> matBufferOffset_ps;
	std::vector<uint32_t> matBufferSize_ps;
	uint32_t objectUboOffset = 0;
	uint32_t textureDescriptorCount = 0;
	_pipelineTemps.clear();
	auto frameIndex = _renderer->GetCurrentFrameIndex();
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
		uint32_t modelPrimitiveCount = 0;
		uint64_t vbPos = 0;
		uint64_t ibPos = 0;
		bool bNeedUpdateObjUniform = false;
		bool bNeedUpdateVbIb = false;
		bool bBeginUpdate = false;
		//更新缓冲区
		for (auto m : *matPrim)
		{
			auto pipelineObj = PipelineManager::GetGraphicsPipelineMap(m->_graphicsIndex);
			//Get Pipeline
			{
				if (pipelineObj == nullptr)
				{
					HString vsShaderFullName = m->_graphicsIndex.GetVSShaderFullName();
					HString psShaderFullName = m->_graphicsIndex.GetPSShaderFullName();
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
					PipelineManager::SetVertexInput(pipelineCreateInfo, m->_inputLayout);
					PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache.get(), psCache.get());
					//Setting pipeline end
					VkPipelineLayout pipelineLayout;
					if (pipelineCreateInfo.bHasMaterialTexture)
					{
						if (pipelineCreateInfo.bHasMaterialParameterVS && !pipelineCreateInfo.bHasMaterialParameterPS)
							pipelineLayout = PipelineManager::GetPipelineLayout_P_O_VSM_T(psCache->header.shaderTextureCount);
						else if (pipelineCreateInfo.bHasMaterialParameterPS && !pipelineCreateInfo.bHasMaterialParameterVS)
							pipelineLayout = PipelineManager::GetPipelineLayout_P_O_PSM_T(psCache->header.shaderTextureCount);
						else if (pipelineCreateInfo.bHasMaterialParameterPS && pipelineCreateInfo.bHasMaterialParameterVS)
							pipelineLayout = PipelineManager::GetPipelineLayout_P_O_VSPSM_T(psCache->header.shaderTextureCount);
						else
							pipelineLayout = PipelineManager::GetPipelineLayout_P_O_T(psCache->header.shaderTextureCount);
					}
					else
					{
						if (pipelineCreateInfo.bHasMaterialParameterVS && !pipelineCreateInfo.bHasMaterialParameterPS)
							pipelineLayout = PipelineManager::GetPipelineLayout_P_O_VSM();
						else if (pipelineCreateInfo.bHasMaterialParameterPS && !pipelineCreateInfo.bHasMaterialParameterVS)
							pipelineLayout = PipelineManager::GetPipelineLayout_P_O_PSM();
						else if (pipelineCreateInfo.bHasMaterialParameterPS && pipelineCreateInfo.bHasMaterialParameterVS)
							pipelineLayout = PipelineManager::GetPipelineLayout_P_O_VSPSM();
						else
							pipelineLayout = PipelineManager::GetPipelineLayout_P_O();
					}
					pipelineObj = PipelineManager::CreatePipelineObject(pipelineCreateInfo, pipelineLayout,
						_renderPass, m->_graphicsIndex, (uint32_t)_subpassDescs.size());
				}

				if (_pipelineTemps.capacity() <= _pipelineTemps.size())
				{
					if (_pipelineTemps.capacity() < 200)
						_pipelineTemps.reserve(_pipelineTemps.capacity() + 10);
					else
						_pipelineTemps.reserve(_pipelineTemps.capacity() * 2);
				}
				_pipelineTemps.push_back(pipelineObj);
			}

			auto matPrimGroup = PrimitiveProxy::GetMaterialPrimitiveGroup(m, _renderer);
			if (matPrimGroup)
			{
				//Material uniform buffer
				{
					bool bNeedUpdateVSUniformBuffer = false;
					bool bNeedUpdatePSUniformBuffer = false;
					bool bNeedUpdateTextures = false;
					matPrimGroup->ResetDecriptorSet(pipelineObj->numTextures, bNeedUpdateVSUniformBuffer, bNeedUpdatePSUniformBuffer, bNeedUpdateTextures);
					matPrimGroup->UpdateDecriptorSet(bNeedUpdateVSUniformBuffer, bNeedUpdatePSUniformBuffer, bNeedUpdateTextures);
				}

				//Primtive/Vertex/Index buffer	
				{
					modelPrimitiveCount += matPrimGroup->prims.size();//计算PrimitiveCount
					//更新Primitives
					for (auto& prim : (matPrimGroup->prims))
					{
						if (prim->bNeedUpdate)
						{
							prim->bNeedUpdate = false;
							bBeginUpdate = true;
						}

						VkDeviceSize currentBufferWholeSize_vb = vbPos + prim->vbSize;
						VkDeviceSize currentBufferWholeSize_ib = ibPos + prim->ibSize;
						uint32_t currentObjectUniformBufferWholeSize = objectUboOffset + (uint32_t)sizeof(ObjectUniformBuffer);
						//检查buffer大小是否充足
						{
							//obj
							VkDeviceSize targetBufferSize_objub = ((VkDeviceSize)currentObjectUniformBufferWholeSize + (VkDeviceSize)UniformBufferSizeRange - 1) & ~((VkDeviceSize)UniformBufferSizeRange - 1);
							bNeedUpdateObjUniform = obj->ResizeDescriptorTargetBuffer(currentObjectUniformBufferWholeSize, targetBufferSize_objub);
							//vb
							VkDeviceSize targetBufferSize_vb = (currentBufferWholeSize_vb + (VkDeviceSize)BufferSizeRange - 1) & ~((VkDeviceSize)BufferSizeRange - 1);
							bNeedUpdateVbIb |= vb->Resize(targetBufferSize_vb);
							//ib
							VkDeviceSize targetBufferSize_ib = (currentBufferWholeSize_ib + (VkDeviceSize)BufferSizeRange - 1) & ~((VkDeviceSize)BufferSizeRange - 1);
							bNeedUpdateVbIb |= ib->Resize(targetBufferSize_ib);
						}

						//拷贝数据
						if (bBeginUpdate || prim->transform->NeedUpdateUb() || bNeedUpdateObjUniform)
						{
							ObjectUniformBuffer objectUniformBuffer = {};
							objectUniformBuffer.WorldMatrix = prim->transform->GetWorldMatrix();
							obj->BufferMapping(&objectUniformBuffer, (uint64_t)objectUboOffset, sizeof(ObjectUniformBuffer));
						}
						if (bBeginUpdate || bNeedUpdateVbIb)
						{
							vb->BufferMapping(prim->vertexData.data(), vbPos, prim->vbSize);
							ib->BufferMapping(prim->vertexIndices.data(), ibPos, prim->ibSize);
						}

						//更新buffer起始位置
						vbPos = currentBufferWholeSize_vb;
						ibPos = currentBufferWholeSize_ib;
						objectUboOffset = currentObjectUniformBufferWholeSize;
					}
				}
			}

		}
		//更新obj的描述符集大小，每个obj对应一个uniform大小，所以只更新一次就行了。
		obj->UpdateDescriptorSet(sizeof(ObjectUniformBuffer), 0);
	}

	//Update Render
	objectUboOffset = 0;
	uint32_t dynamicOffset[] = { 0 };
	uint32_t matIndex = 0;
	uint32_t primIndex = 0;
	VkDeviceSize vbOffset = 0;
	VkDeviceSize ibOffset = 0;
	auto matPrims = PrimitiveProxy::GetMaterialPrimitives((uint32_t)p);
	for (auto m : *matPrims)
	{
		const auto matPrimGroup = PrimitiveProxy::GetMaterialPrimitiveGroup(m, _renderer);
		if (matPrimGroup && matPrimGroup->prims.size() > 0)
		{
			auto curPipeline = _pipelineTemps[matIndex];
			//Pass uniform buffers
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 0, 1, &pass->GetDescriptorSet(), 1, dynamicOffset);
			}
			manager->CmdCmdBindPipeline(cmdBuf, curPipeline->pipeline, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS);
			//Material uniform buffers & Textures
			if (curPipeline->bHasMaterialParameterVS && curPipeline->bHasMaterialParameterPS && curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &matPrimGroup->descriptorSet_uniformBufferVS->GetDescriptorSet(), 1, dynamicOffset);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &matPrimGroup->descriptorSet_uniformBufferPS->GetDescriptorSet(), 1, dynamicOffset);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 4, 1, &matPrimGroup->descriptorSet_texture[frameIndex], 0, nullptr);
			}
			else if (curPipeline->bHasMaterialParameterVS && curPipeline->bHasMaterialParameterPS && !curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &matPrimGroup->descriptorSet_uniformBufferVS->GetDescriptorSet(), 1, dynamicOffset);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &matPrimGroup->descriptorSet_uniformBufferPS->GetDescriptorSet(), 1, dynamicOffset);
			}
			else if (curPipeline->bHasMaterialParameterVS && !curPipeline->bHasMaterialParameterPS && !curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &matPrimGroup->descriptorSet_uniformBufferVS->GetDescriptorSet(), 1, dynamicOffset);
			}
			else if (!curPipeline->bHasMaterialParameterVS && curPipeline->bHasMaterialParameterPS && !curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &matPrimGroup->descriptorSet_uniformBufferPS->GetDescriptorSet(), 1, dynamicOffset);
			}
			else if (!curPipeline->bHasMaterialParameterVS && curPipeline->bHasMaterialParameterPS && curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &matPrimGroup->descriptorSet_uniformBufferPS->GetDescriptorSet(), 1, dynamicOffset);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &matPrimGroup->descriptorSet_texture[frameIndex], 0, nullptr);
			}
			else if (curPipeline->bHasMaterialParameterVS && !curPipeline->bHasMaterialParameterPS && curPipeline->bHasMaterialTexture)
			{
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 2, 1, &matPrimGroup->descriptorSet_uniformBufferVS->GetDescriptorSet(), 1, dynamicOffset);
				vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 3, 1, &matPrimGroup->descriptorSet_texture[frameIndex], 0, nullptr);
			}
			for (auto& prim : matPrimGroup->prims)
			{
				if (prim->renderer == this->_renderer)
				{
					vkCmdBindDescriptorSets(cmdBuf, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, curPipeline->layout, 1, 1, &obj->GetDescriptorSet(), 1, &objectUboOffset);
					objectUboOffset += sizeof(ObjectUniformBuffer);
					VkBuffer verBuf[] = { vb->GetBuffer() };
					vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);
					vkCmdBindIndexBuffer(cmdBuf, ib->GetBuffer(), ibOffset, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(cmdBuf, (uint32_t)(prim->ibSize / sizeof(uint32_t)), 1, 0, 0, 0);
					vbOffset += prim->vbSize;
					ibOffset += prim->ibSize;
				}
				primIndex++;
			}
		}
		matIndex++;
	}
}

#pragma endregion BasePass
