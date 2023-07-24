#include "PassBase.h"
#include "VulkanRenderer.h"
#include "Texture.h"
#include "PassManager.h"
#include "VertexFactory.h"
void GraphicsPass::BuildPass()
{
	VulkanRenderer::GetManager()->CreateRenderPass(_attachmentDescs, _subpassDependencys, _subpassDescs, _renderPass);
	_pipeline.reset(new Pipeline());
	//Setting pipeline start
	//.....
	//Setting pipeline end
	_pipeline->CreatePipelineObject(_renderPass, _subpassDescs.size());
}

void GraphicsPass::AddAttachment(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,std::shared_ptr<Texture> texture )
{
	//attachment
	VkAttachmentDescription attachmentDesc = {} ;

	attachmentDesc.flags = 0;
	attachmentDesc.format = texture->GetFormat();
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.loadOp = loadOp;
	attachmentDesc.storeOp = storeOp;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.initialLayout = texture->GetLayout();

	if (texture->GetLayout() == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	else if (texture->GetUsageFlags() & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	else if (texture->GetUsageFlags() & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		if (texture->GetAspectFlags() & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		{
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}
	}
	_attachmentDescs.push_back(attachmentDesc);
}

void GraphicsPass::AddSubpass(std::vector<uint32_t> inputIndexes, std::vector<uint32_t> colorIndexes, int depthStencilIndex)
{
	_input_ref.resize(inputIndexes.size());
	for (int i = 0; i < inputIndexes.size(); i++)
	{
		_input_ref[i].attachment = inputIndexes[i];
		_input_ref[i].layout = _attachmentDescs[inputIndexes[i]].finalLayout;
	}
	_color_ref.resize(colorIndexes.size());
	for (int i = 0; i < colorIndexes.size(); i++)
	{
		_color_ref[i].attachment = colorIndexes[i];
		_color_ref[i].layout = _attachmentDescs[colorIndexes[i]].finalLayout;
	}
	if (depthStencilIndex >= 0)
	{
		_depthStencil_ref.attachment = depthStencilIndex;
		_depthStencil_ref.layout = _attachmentDescs[depthStencilIndex].finalLayout;
	}
	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.flags = 0;
	subpassDesc.inputAttachmentCount = _input_ref.size();
	subpassDesc.pInputAttachments = _input_ref.data();
	subpassDesc.colorAttachmentCount = _color_ref.size();
	subpassDesc.pColorAttachments = _color_ref.data();
	subpassDesc.pResolveAttachments = NULL;
	subpassDesc.pDepthStencilAttachment = depthStencilIndex >= 0 ? &_depthStencil_ref : NULL;
	subpassDesc.preserveAttachmentCount = 0;
	subpassDesc.pPreserveAttachments = NULL;
	//
	_subpassDescs.push_back(subpassDesc);

	VkSubpassDependency depen = {};
	if (_subpassDependencys.size() <= 0)//The srcSubpass  of the first subpass should be outside renderpass.It looks like it could be 0.
	{
		depen.srcSubpass = VK_SUBPASS_EXTERNAL;//0
		depen.dstSubpass = 1;//Next subpass.
		depen.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		if(depthStencilIndex >= 0)
		{
			depen.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			depen.dstAccessMask =
				VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			depen.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ;
			depen.dstAccessMask =
				VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}	
		depen.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		depen.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	_subpassDependencys.push_back(depen);
}

std::shared_ptr<Texture> PassBase::GetSceneTexture(uint32_t descIndex)
{
	return _renderer->GetPassManager()->GetSceneTexture()->GetTexture(SceneTextureDesc(descIndex));
}


void OpaquePass::PassInit()
{
	//Swapchain
	AddAttachment(VK_ATTACHMENT_LOAD_OP_CLEAR , VK_ATTACHMENT_STORE_OP_STORE,_renderer->GetSwapchainImage());
	AddSubpass({}, {0}, {});
	BuildPass();
}

void OpaquePass::BuildPass()
{
	VulkanRenderer::GetManager()->CreateRenderPass(_attachmentDescs, _subpassDependencys, _subpassDescs, _renderPass);
	_pipeline.reset(new Pipeline());
	//Setting pipeline start
	_pipeline->SetColorBlend(false);
	_pipeline->SetRenderRasterizer();
	_pipeline->SetRenderDepthStencil();
	_pipeline->SetVertexInput(VertexFactory::VertexInputBase::BuildLayout());
	_pipeline->SetPipelineLayout({});
	_pipeline->SetVertexShaderAndPixelShader();
	//Setting pipeline end
	_pipeline->CreatePipelineObject(_renderPass, _subpassDescs.size());
}