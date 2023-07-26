#include "PassManager.h"
#include "VulkanRenderer.h"
#include "PassBase.h"

void PassManager::PassesInit(VulkanRenderer* renderer)
{
	_renderer = renderer;
	_sceneTextures.reset(new SceneTexture(renderer));
	{
		//Opaque Pass
		std::shared_ptr<OpaquePass> opaque = std::make_shared<OpaquePass>(renderer);
		AddPass(opaque, "Opaque");

	}
	for (auto p : _passes)
	{
		p.second->PassInit();
	}
}

void PassManager::PassesUpdate()
{
	_sceneTextures->UpdateTextures();
	//Collect render setting (Commandbuffer record)
	std::vector<std::shared_ptr<PassBase>>executePasses;
	for (auto p : _passes)
	{
		p.second->PassUpdate();
		executePasses.push_back(p.second);
	}
	//Execute
	VulkanManager::GetManager()->SubmitQueueForPasses(executePasses, _renderer->GetPresentSemaphore());
}

void PassManager::PassesRelease()
{
	_sceneTextures.reset();
}

void PassManager::AddPass(std::shared_ptr<PassBase> newPass, const char* passName)
{
	if (newPass == NULL)
	{
		MessageOut("Add Pass Failed.The New Pass Is Null.", true, true);
	}
	auto it = _passes.find(passName);
	if (it != _passes.end())
	{
		MessageOut("Add Pass Failed.Pass Name Has Been Exist.", true, true);
	}
	newPass->_passName = passName;
	_passes.emplace(std::make_pair(passName, newPass));
}

SceneTexture::SceneTexture(VulkanRenderer* renderer)
{
	auto sceneColor = Texture::CreateTexture2D(1, 1, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneColor");
	auto sceneDepth = Texture::CreateTexture2D(1, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneDepth");
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneColor, sceneColor));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::DepthStencil, sceneDepth));
}

void SceneTexture::UpdateTextures()
{
}
