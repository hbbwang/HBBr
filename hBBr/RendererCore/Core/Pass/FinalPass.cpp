#include "FinalPass.h"
#include "VulkanRenderer.h"
#include "Texture2D.h"

//最终pass，把final rt复制到swapchain上

FinalPass::~FinalPass()
{

}

void FinalPass::PassInit()
{
	_passName = "Final Pass";
	_markColor = glm::vec4(0.8, 0.8, 0.9, 0.6);
}

void FinalPass::PassUpdate()
{
	const auto& manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();

	int swapchainIndex = _renderer->GetCurrentFrameIndex();
	auto swapchainImage = _renderer->GetSwapchainImages()[swapchainIndex];
	auto finalRT = GetSceneTexture(SceneTextureDesc::FinalColor);

	manager->Transition(cmdBuf, swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	finalRT->Transition(cmdBuf, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	ColorBitImage(cmdBuf, finalRT->GetTexture(), swapchainImage, _renderer->GetRenderSize(), _renderer->GetWindowSurfaceSize());
	finalRT->Transition(cmdBuf, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	manager->Transition(cmdBuf, swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}
