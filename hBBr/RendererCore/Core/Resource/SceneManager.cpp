#include"SceneManager.h"
#include "VulkanRenderer.h"

SceneManager::~SceneManager()
{

}

void SceneManager::SceneInit(class VulkanRenderer* renderer)
{
	_renderer = renderer;
}

void SceneManager::SceneUpdate()
{
	//Update Objecet
	const auto objCount = _gameObjects.size();
	for (int i = 0; i < objCount; i++)
	{
		if (!_gameObjects[i]->Update())
		{
			i -= 1;
		}
	}
}
