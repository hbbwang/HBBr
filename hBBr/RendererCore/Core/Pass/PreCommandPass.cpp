#include "PreCommandPass.h"
#include "VulkanRenderer.h"

PreCommandPass::~PreCommandPass()
{
}

void PreCommandPass::PassInit()
{
	_passName = "Precommand Pass";
	_markColor = glm::vec4(1, 1, 0, 0.5);
}

void PreCommandPass::PassUpdate()
{
	const auto cmdBuf = _renderer->GetCommandBuffer();
	//Image data CPU to GPU
	auto& uploadTexs = Texture2D::GetUploadTextures();
	for (auto it = uploadTexs.begin(); it != uploadTexs.end(); it++)
	{
		if ((*it)->CopyBufferToTexture(cmdBuf))
		{
			it = uploadTexs.erase(it) - 1;
		}
	}
}
