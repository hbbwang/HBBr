#include"SceneManager.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "FileSystem.h"
SceneManager::~SceneManager()
{
	//wait gameobject destroy
	while (_gameObjects.size() > 0)
	{
		for (int i = 0; i < _gameObjects.size(); i++)
		{
			_gameObjects[i]->Destroy();
			_gameObjects[i]->Update();
		}
	}
}

void SceneManager::SceneInit(class VulkanRenderer* renderer)
{
	_renderer = renderer;
	//Test
	GameObject* cube = new GameObject();
	auto modelComp = cube->AddComponent<ModelComponent>();
	modelComp->SetModel(FileSystem::GetResourceAbsPath() + "Content/FBX/TestFbx_1_Combine.FBX");
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
