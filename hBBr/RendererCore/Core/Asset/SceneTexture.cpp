#include "SceneTexture.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
#include "Texture2D.h"

SceneTexture::SceneTexture(VulkanRenderer* renderer)
{
	auto manager = VulkanManager::GetManager();
	_renderer = renderer;
	auto sceneColor = Texture2D::CreateTexture2D(1, 1, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneColor");
	auto sceneDepth = Texture2D::CreateTexture2D(1, 1, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneDepth");
	auto finalColor = Texture2D::CreateTexture2D(1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, "FinalColor");
	//Transition
	VkCommandBuffer cmdbuf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), cmdbuf);
	manager->BeginCommandBuffer(cmdbuf);
	{
		sceneColor->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		sceneDepth->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		finalColor->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	manager->EndCommandBuffer(cmdbuf);
	manager->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(manager->GetGraphicsQueue());
	manager->FreeCommandBuffer(manager->GetCommandPool(), cmdbuf);
	//
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneColor, sceneColor));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneDepth, sceneDepth));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::FinalColor, finalColor));
}

void SceneTexture::UpdateTextures()
{
	if (_sceneTexture.size() <= 0)
	{
		return;
	}
	auto size = _renderer->GetRenderSize();
	auto sceneDepth = _sceneTexture[SceneTextureDesc::SceneDepth];
	if (sceneDepth->GetTextureSize().width != size.width ||
		sceneDepth->GetTextureSize().height != size.height)
	{
		sceneDepth->Resize(size.width, size.height);
	}
	auto finalColor = _sceneTexture[SceneTextureDesc::FinalColor];
	if (finalColor->GetTextureSize().width != size.width ||
		finalColor->GetTextureSize().height != size.height)
	{
		finalColor->Resize(size.width, size.height);
	}
}