#include "PassManager.h"
#include "VulkanRenderer.h"
#include "PassBase.h"

void PassManager::PassesInit()
{
	_sceneTextures.reset(new SceneTexture);
	for (auto p : _passes)
	{
		p.second->PassInit();
	}
}

void PassManager::PassesUpdate()
{
	for (auto p : _passes)
	{
		p.second->PassUpdate();
	}
}

void PassManager::PassesRelease()
{
	_sceneTextures.reset();
}

