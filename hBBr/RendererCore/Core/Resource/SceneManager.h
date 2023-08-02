#pragma once

//场景类,每个渲染器必须有一个场景管理类,用来储存当前场景的所有对象(GameObject)
#include <vector>
#include "Component/GameObject.h"

class SceneManager
{
	friend class VulkanRenderer;
	friend class GameObject;
public:

	~SceneManager();

private:

	void SceneInit(class VulkanRenderer* renderer);

	void SceneUpdate();

	class VulkanRenderer* _renderer = NULL;

	std::vector<GameObject*> _gameObjects;

	//需要进行父子关系处理的对象
	std::vector<GameObject*> _gameObjectParentSettings;

	//需要进行销毁的Objects
	std::vector<GameObject*> _gameObjectNeedDestroy;
};



