#include "PassBase.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
#include "Texture2D.h"
#include "PassManager.h"
#include "VertexFactory.h"
#include "DescriptorSet.h"

PassBase::PassBase(VulkanRenderer* renderer)
{
	_renderer = renderer;
}

PassBase::~PassBase()
{

}

std::shared_ptr<Texture2D> PassBase::GetSceneTexture(uint32_t descIndex)
{
	return _renderer->GetPassManager()->GetSceneTexture()->GetTexture(SceneTextureDesc(descIndex));
}

void GraphicsPass::ResetFrameBuffer(VkExtent2D size, std::vector<VkImageView> imageViews)
{
	if (_currentFrameBufferSize.width != size.width || _currentFrameBufferSize.height != size.height)
	{
		_currentFrameBufferSize = size;
		const auto manager = VulkanManager::GetManager();
		//const auto frameIndex = VulkanRenderer::GetCurrentFrameIndex();
		//Insert swapchain imageView to first.
		if (_framebuffers.size() > 0)
		{
			vkQueueWaitIdle(manager->GetGraphicsQueue());
			manager->DestroyFrameBuffers(_framebuffers);
		}
		_framebuffers.resize(manager->GetSwapchainBufferCount());
		//VulkanManager::GetManager()->CreateFrameBuffer(size.width, size.height, _renderPass, imageViews, _framebuffers[frameIndex]);
		//VulkanManager::GetManager()->CreateFrameBuffers({ size.width, size.height }, _renderPass, imageViews, _framebuffers);
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			std::vector<VkImageView> ivs = { _renderer->GetSwapchainImageViews()[i] };
			if (imageViews.size() > 0)
			{
				ivs.insert(ivs.end(), imageViews.begin(), imageViews.end());
			}
			VulkanManager::GetManager()->CreateFrameBuffer(size.width, size.height, _renderPass, ivs, _framebuffers[i]);
		}
	}
}

void GraphicsPass::ResetFrameBufferCustom(VkExtent2D size, std::vector<VkImageView> imageViews)
{
	if (_currentFrameBufferSize.width != size.width || _currentFrameBufferSize.height != size.height)
	{
		_currentFrameBufferSize = size;
		const auto manager = VulkanManager::GetManager();
		//const auto frameIndex = VulkanRenderer::GetCurrentFrameIndex();
		//Insert swapchain imageView to first.
		if (_framebuffers.size() > 0)
		{
			vkQueueWaitIdle(manager->GetGraphicsQueue());
			manager->DestroyFrameBuffers(_framebuffers);
		}
		_framebuffers.resize(manager->GetSwapchainBufferCount());
		for (int i = 0; i < _framebuffers.size(); i++)
		{
			VulkanManager::GetManager()->CreateFrameBuffer(size.width, size.height, _renderPass, imageViews, _framebuffers[i]);
		}
	}
}

void GraphicsPass::PassUpdate()
{
	const auto manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.3, 1.0, 0.1, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), { GetSceneTexture((uint32_t)SceneTextureDesc::SceneDepth)->GetTextureView() });
	SetViewport(_currentFrameBufferSize);
	BeginRenderPass({ 0,0,0,0 });
	//Begin...
	PassRender();
	//End...
	EndRenderPass();
}

void GraphicsPass::CreateRenderPass()
{
	VulkanManager::GetManager()->CreateRenderPass(_attachmentDescs, _subpassDependencys, _subpassDescs, _renderPass);
}

GraphicsPass::~GraphicsPass()
{
	VulkanManager::GetManager()->DestroyFrameBuffers(_framebuffers);
	VulkanManager::GetManager()->DestroyRenderPass(_renderPass);
}

void GraphicsPass::AddAttachment(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkFormat attachmentFormat, VkImageLayout initLayout, VkImageLayout finalLayout)
{
	//attachment
	VkAttachmentDescription attachmentDesc = {};
	attachmentDesc.flags = 0;
	attachmentDesc.format = attachmentFormat;
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.loadOp = loadOp;
	attachmentDesc.storeOp = storeOp;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.initialLayout = initLayout;
	attachmentDesc.finalLayout = finalLayout;
	_attachmentDescs.push_back(attachmentDesc);
}

void GraphicsPass::AddSubpass(std::vector<uint32_t> inputIndexes, std::vector<uint32_t> colorIndexes, int depthStencilIndex)
{
	if (depthStencilIndex >= 0)
	{
		AddSubpass(inputIndexes, colorIndexes, depthStencilIndex,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			);
	}
	else
	{
		AddSubpass(inputIndexes, colorIndexes, depthStencilIndex,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		);
	}

}

void GraphicsPass::AddSubpass(std::vector<uint32_t> inputIndexes, std::vector<uint32_t> colorIndexes, int depthStencilIndex,
	VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	_input_ref.resize(inputIndexes.size());
	for (int i = 0; i < inputIndexes.size(); i++)
	{
		_input_ref[i].attachment = inputIndexes[i];
		//_input_ref[i].layout = _attachmentDescs[inputIndexes[i]].finalLayout;
		_input_ref[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	}
	_color_ref.resize(colorIndexes.size());
	for (int i = 0; i < colorIndexes.size(); i++)
	{
		_color_ref[i].attachment = colorIndexes[i];
		//_color_ref[i].layout = _attachmentDescs[colorIndexes[i]].finalLayout;
		_color_ref[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (depthStencilIndex >= 0)
	{
		_depthStencil_ref.attachment = depthStencilIndex;
		//_depthStencil_ref.layout = _attachmentDescs[depthStencilIndex].finalLayout;
		_depthStencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.flags = 0;
	subpassDesc.inputAttachmentCount = (uint32_t)_input_ref.size();
	subpassDesc.pInputAttachments = _input_ref.data();
	subpassDesc.colorAttachmentCount = (uint32_t)_color_ref.size();
	subpassDesc.pColorAttachments = _color_ref.data();
	subpassDesc.pResolveAttachments = nullptr;
	subpassDesc.pDepthStencilAttachment = depthStencilIndex >= 0 ? &_depthStencil_ref : nullptr;
	subpassDesc.preserveAttachmentCount = 0;
	subpassDesc.pPreserveAttachments = nullptr;
	//
	_subpassDescs.push_back(subpassDesc);

	VkSubpassDependency depen = {};
	if (_subpassDependencys.size() <= 0)
	{
		depen.srcSubpass = VK_SUBPASS_EXTERNAL;//pass out side
		depen.dstSubpass = 0;//Next subpass,into the first subpass.	
	}
	else
	{
		depen.srcSubpass = (uint32_t)_subpassDependencys.size() - 1;
		depen.dstSubpass = (uint32_t)_subpassDependencys.size();//Next subpass.	
	}
	depen.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	depen.srcAccessMask = srcAccessMask;
	depen.dstAccessMask = dstAccessMask;
	depen.srcStageMask = srcStageMask;
	depen.dstStageMask = dstStageMask;
	_subpassDependencys.push_back(depen);
}

void GraphicsPass::BeginRenderPass(std::array<float, 4> clearColor, VkExtent2D areaSize)
{
	if (areaSize.width <= 0 && areaSize.height <= 0)
	{
		areaSize = _currentFrameBufferSize;
	}
	VulkanManager::GetManager()->BeginRenderPass(_renderer->GetCommandBuffer(), GetFrameBuffer(), _renderPass, areaSize, _attachmentDescs, clearColor);
}

void GraphicsPass::EndRenderPass()
{
	VulkanManager::GetManager()->EndRenderPass(_renderer->GetCommandBuffer());
}

void GraphicsPass::SetViewport(VkExtent2D viewportSize)
{
	VulkanManager::GetManager()->CmdSetViewport(_renderer->GetCommandBuffer(), { viewportSize });
}

VkFramebuffer GraphicsPass::GetFrameBuffer()const
{
	return _framebuffers[_renderer->GetCurrentFrameIndex()];
}
