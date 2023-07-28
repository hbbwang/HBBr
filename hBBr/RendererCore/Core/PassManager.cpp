#include "PassManager.h"
#include "VulkanRenderer.h"
#include "PassBase.h"

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

void PassManager::PassesInit(VulkanRenderer* renderer)
{
	_renderer = renderer;
	_sceneTextures.reset(new SceneTexture(renderer));
	{
		//Opaque Pass
		std::shared_ptr<OpaquePass> opaque = std::make_shared<OpaquePass>(renderer);
		AddPass(opaque, "Opaque");
		std::shared_ptr<ImguiPass> imgui = std::make_shared<ImguiPass>(renderer);
		AddPass(imgui, "Imgui");
	}
	if (_executeFence.size() <= 0)
	{
		VulkanManager::GetManager()->CreateRenderFences(_executeFence);
	}
	for (auto p : _passes)
	{
		p->PassInit();
		p->PassBuild();
	}
}

void PassManager::PassesUpdate()
{
	const auto manager = VulkanManager::GetManager();

	const uint32_t frameIndex = VulkanRenderer::GetCurrentFrameIndex();

	manager->WaitForFences({ _executeFence[frameIndex] });

	_sceneTextures->UpdateTextures();

	manager->BeginCommandBuffer(_renderer->GetCommandBuffer());
	//Collect render setting (Commandbuffer record)
	for (auto p : _passes)
	{
		p->PassUpdate();

		_executePasses.push_back(p);
	}
	manager->EndCommandBuffer(_renderer->GetCommandBuffer());

	//Execute
	manager->SubmitQueueForPasses(_renderer->GetCommandBuffer(), _executePasses, _renderer->GetPresentSemaphore(), _renderer->GetSubmitSemaphore(), _executeFence[frameIndex]);
}

void PassManager::PassesRelease()
{
	VulkanManager::GetManager()->DestroyRenderFences(_executeFence);
	_passes.clear();
	_sceneTextures.reset();
}

void PassManager::PassesReset()
{
	for (auto& p : _passes)
	{
		p->PassReset();
	}
}

void PassManager::AddPass(std::shared_ptr<PassBase> newPass, const char* passName)
{
	if (newPass == NULL)
	{
		MessageOut("Add Pass Failed.The New Pass Is Null.", true, true);
	}
	auto it = std::find_if(_passes.begin(), _passes.end(), [newPass](std::shared_ptr<PassBase> &inPass) {
		return inPass->GetName() == newPass->GetName();
	});
	if (it != _passes.end())
	{
		MessageOut("Add Pass Failed.Pass Name Has Been Exist.", true, true);
	}
	newPass->_passName = passName;
	_passes.push_back(newPass);
}
