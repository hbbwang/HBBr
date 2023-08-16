#pragma once

//场景类,每个渲染器必须有一个场景管理类,用来储存当前场景的所有对象(GameObject)
#include <vector>
#include <memory>
#include <functional>
#include "Component/GameObject.h"

typedef void (*EditorSceneUpdate)(class SceneManager* scene, std::vector<std::shared_ptr<GameObject>> aliveObjects);

typedef void (*EditorGameObjectAdd)(class SceneManager* scene, std::shared_ptr<GameObject> newObject);

typedef void (*EditorGameObjectRemove)(class SceneManager* scene, std::shared_ptr<GameObject> oldObject);

class SceneManager
{
	friend class VulkanRenderer;
	friend class GameObject;
public:

	~SceneManager();

#if IS_EDITOR
	//EditorSceneUpdate _editorSceneUpdateFunc = NULL;
	//EditorGameObjectAdd _editorGameObjectAddFunc = NULL;
	//EditorGameObjectRemove _editorGameObjectRemoveFunc = NULL;
	std::function<void(class SceneManager*, std::vector<std::shared_ptr<GameObject>>)> _editorSceneUpdateFunc = [](class SceneManager* s, std::vector<std::shared_ptr<GameObject>> o) {};
	std::function<void(class SceneManager*, std::shared_ptr<GameObject>)> _editorGameObjectAddFunc = [](class SceneManager* scene, std::shared_ptr<GameObject> newObject) {};
	std::function<void(class SceneManager*, std::shared_ptr<GameObject>)> _editorGameObjectRemoveFunc = [](class SceneManager* scene, std::shared_ptr<GameObject> oldObject) {};
	std::function<void(class SceneManager*, std::shared_ptr<GameObject>)> _editorGameObjectUpdateFunc = [](class SceneManager* scene, std::shared_ptr<GameObject> oldObject) {};
#endif

private:

	void SceneInit(class VulkanRenderer* renderer);

	void SceneUpdate();

	void AddNewObject(std::shared_ptr<GameObject> newObject);

	void RemoveObject(GameObject* object);

	class VulkanRenderer* _renderer = NULL;

	std::vector<std::shared_ptr<GameObject>> _gameObjects;

	//需要进行销毁的Objects
	std::vector<std::shared_ptr<GameObject>> _gameObjectNeedDestroy;
};



