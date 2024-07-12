#include "PreCommandPass.h"
#include "VulkanRenderer.h"

PreCommandPass::~PreCommandPass()
{
	const auto& manager = VulkanManager::GetManager();
	manager->FreeCommandBuffer(manager->GetCommandPool(), _cmdBuf);
}

void PreCommandPass::PassInit()
{
	_passName = "Precommand Pass";
	_markColor = glm::vec4(1, 1, 0, 0.5);

	const auto& manager = VulkanManager::GetManager();
	manager->AllocateCommandBuffer(manager->GetCommandPool(), _cmdBuf);
}

void PreCommandPass::PassUpdate()
{
	const auto& manager = VulkanManager::GetManager();
	bool bNeedSubmitQueue = false;
	if (Texture2D::GetUploadTextures().size() > 0)
	{
		bNeedSubmitQueue = true;
	}

	if (bNeedSubmitQueue)
	{
		manager->ResetCommandBuffer(_cmdBuf);
		manager->BeginCommandBuffer(_cmdBuf, 0);
		{
			//Image data CPU to GPU
			for (auto ut : Texture2D::GetUploadTextures())
			{
				ut->CopyBufferToTexture(_cmdBuf);
			}
		}
		manager->EndCommandBuffer(_cmdBuf);
		manager->SubmitQueueImmediate({ _cmdBuf });//确保纹理立刻加载到显存

		for (auto ut : Texture2D::GetUploadTextures())
		{
			ut->DestoryUploadBuffer();
		}
		Texture2D::GetUploadTextures().clear();
	}
	
}
