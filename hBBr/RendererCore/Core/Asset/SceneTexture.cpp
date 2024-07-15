#include "SceneTexture.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
#include "Texture2D.h"

SceneTexture::SceneTexture(VulkanRenderer* renderer)
{
	auto manager = VulkanManager::GetManager();

	_renderer = renderer;
	auto finalColor =	Texture2D::CreateTexture2D(1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, "FinalColor");
	//SceneDepth
	auto sceneDepth =	Texture2D::CreateTexture2D(1, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneDepth");
	//SceneColor Emissive
	auto sceneColor =	Texture2D::CreateTexture2D(1, 1, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneColor");
	//(RGB)BaseColor,(A)Roughness
	auto gBuffer0 =		Texture2D::CreateTexture2D(1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "GBuffer0");
	//(RGB)WorldNormal,(A)只有两位,可以用来存4个状态
	auto gBuffer1 =		Texture2D::CreateTexture2D(1, 1, VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "GBuffer1");
	//(R)Metallic,(G)Specular,(B)Material AO,(A)ShadingModel ID
	auto gBuffer2 =		Texture2D::CreateTexture2D(1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "GBuffer2");

	//Transition
	VkCommandBuffer cmdbuf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), cmdbuf);
	manager->BeginCommandBuffer(cmdbuf);
	{
		sceneColor->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		sceneDepth->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		finalColor->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		gBuffer0->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		gBuffer1->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		gBuffer2->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	manager->EndCommandBuffer(cmdbuf);
	manager->SubmitQueueImmediate({ cmdbuf });
	manager->FreeCommandBuffer(manager->GetCommandPool(), cmdbuf);
	//
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneColor, sceneColor));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneDepth, sceneDepth));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::FinalColor, finalColor));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::GBuffer0, gBuffer0));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::GBuffer1, gBuffer1));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::GBuffer2, gBuffer2));
}

void SceneTexture::UpdateTextures()
{
	if (_sceneTexture.size() <= 0)
	{
		return;
	}
	auto size = _renderer->GetRenderSize();
	for (auto& i : _sceneTexture)
	{
		if (i.second->GetTextureSize().width != size.width ||
			i.second->GetTextureSize().height != size.height)
		{
			i.second->Resize(size.width, size.height);
		}
	}
	//auto sceneDepth = _sceneTexture[SceneTextureDesc::SceneDepth];
	//if (sceneDepth->GetTextureSize().width != size.width ||
	//	sceneDepth->GetTextureSize().height != size.height)
	//{
	//	sceneDepth->Resize(size.width, size.height);
	//}
	//auto finalColor = _sceneTexture[SceneTextureDesc::FinalColor];
	//if (finalColor->GetTextureSize().width != size.width ||
	//	finalColor->GetTextureSize().height != size.height)
	//{
	//	finalColor->Resize(size.width, size.height);
	//}
}