#include "PassManager.h"
#include "VulkanRenderer.h"
#include "PassBase.h"
#include "Pass/PassDefine.h"
#include "Pass/ImguiPass.h"
#include "Pass/GUIPass.h"
void PassManager::PassesInit(VulkanRenderer* renderer)
{
	_renderer = renderer;
	_sceneTextures.reset(new SceneTexture(renderer));
	{
		//Precommand Pass
		std::shared_ptr<PreCommandPass> precommand = std::make_shared<PreCommandPass>(renderer);
		AddPass(precommand, "PreCommand");
		//Opaque Pass
		std::shared_ptr<BasePass> opaque = std::make_shared<BasePass>(renderer);
		AddPass(opaque, "Opaque");
		//Screen GUI Pass
		std::shared_ptr<GUIPass> gui = std::make_shared<GUIPass>(renderer);
		AddPass(gui, "GUI");
#ifdef IS_EDITOR
		std::shared_ptr<ImguiScreenPass> imgui = std::make_shared<ImguiScreenPass>(renderer);
		AddPass(imgui, "Imgui");
#endif
	}
	for (auto p : _passes)
	{
		p->PassInit();
	}
}

void PassManager::PassesUpdate()
{
	const auto manager = VulkanManager::GetManager();

	const uint32_t frameIndex = VulkanRenderer::GetCurrentFrameIndex();

	if (_executeFence.size() != manager->GetSwapchainBufferCount())
	{
		manager->RecreateFences(_executeFence, manager->GetSwapchainBufferCount());
	}

	manager->WaitForFences({ _executeFence[frameIndex] });

	_sceneTextures->UpdateTextures();

	manager->ResetCommandBuffer(_renderer->GetCommandBuffer());
	manager->BeginCommandBuffer(_renderer->GetCommandBuffer());

	//Collect render setting (Commandbuffer record)
	_executePasses.clear();
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
		p->Reset();
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
