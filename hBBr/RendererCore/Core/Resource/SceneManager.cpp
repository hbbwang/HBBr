#include"SceneManager.h"
#include "VulkanRenderer.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Component/CameraComponent.h"
#include "FileSystem.h"
SceneManager::~SceneManager()
{
	//wait gameobject destroy
	while (_gameObjects.size() > 0 || _gameObjectNeedDestroy.size() > 0)
	{
		for (int i = 0; i < _gameObjects.size(); i++)
		{
			_gameObjects[i]->Destroy();
		}
		SceneUpdate();
	}
}

void SceneManager::SceneInit(class VulkanRenderer* renderer)
{
	_renderer = renderer;

#if IS_EDITOR
	//create editor camera
	auto backCamera = new GameObject("EditorCamera", this, true);
	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
	auto cameraComp = backCamera->AddComponent<CameraComponent>();
	cameraComp->OverrideMainCamera();
	_editorCamera = cameraComp;
	_editorCamera->_bIsEditorCamera = true;
#else
	//Test game camera
	auto backCamera = new GameObject("Camera");
	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
	auto cameraComp = backCamera->AddComponent<CameraComponent>();
	cameraComp->OverrideMainCamera();
#endif

	//Test model
	auto test = new GameObject(this);
	auto modelComp0 = test->AddComponent<ModelComponent>();
	modelComp0->SetModelByVirtualPath(FileSystem::GetResourceAbsPath() + "Content/Core/Basic/TestFbx_1_Combine");
	test->SetObjectName("TestFbx_1_Combine");

	GameObject* cube = new GameObject(this);
	testObj = cube->GetSelfWeekPtr();
	auto modelComp = cube->AddComponent<ModelComponent>();
	cube->GetTransform()->SetLocation(glm::vec3(0, 0.5f, 0));
	modelComp->SetModelByVirtualPath(FileSystem::GetResourceAbsPath() + "Content/Core/Basic/Cube");
	cube->SetObjectName("TestFbx_Cube");

}
#include "HInput.h"
void SceneManager::SceneUpdate()
{
	//Test
	if (!testObj.expired() && testObj.lock()->GetTransform())
	{
		testObj.lock()->GetTransform()->SetRotation(testObj.lock()->GetTransform()->GetRotation() + glm::vec3(0, _renderer->GetFrameRate() / 10.0f, 0));
	}

	//Destroy Objects
	//const auto destroyCount = _gameObjectNeedDestroy.size();
	for (auto& i : _gameObjectNeedDestroy)
	{
		i.reset();
	}
	_gameObjectNeedDestroy.clear();

	//Update Objecets
	for (int i = 0; i < _gameObjects.size(); i++)
	{
		if (!_gameObjects[i]->Update())
		{
			i -= 1;
			if (_gameObjects.size() <= 0)
				break;
		}
		else
		{
			#if IS_EDITOR
			if (!_gameObjects[i]->_sceneEditorHide)
				_editorGameObjectUpdateFunc(this, _gameObjects[i]);
			#endif
		}
	}

	//Update Editor if the function is not null.
	#if IS_EDITOR
	_editorSceneUpdateFunc(this, _gameObjects);
	#endif
}

void SceneManager::AddNewObject(std::shared_ptr<GameObject> newObject)
{
	_gameObjects.push_back(newObject);
	#if IS_EDITOR
	if (!newObject->_sceneEditorHide)
	{
		if (!newObject->_sceneEditorHide)
			_editorGameObjectAddFunc(this, newObject);
	}
	#endif
}

void SceneManager::RemoveObject(GameObject* object)
{
	auto it = std::find_if(_gameObjects.begin(),_gameObjects.end(), [object](std::shared_ptr<GameObject>& obj)
		{
			return obj.get() == object;
		});
	if (it != _gameObjects.end())
	{
		//延迟到下一帧再销毁
		_gameObjectNeedDestroy.push_back(*it);
		#if IS_EDITOR
		if(!((*it)->_sceneEditorHide))
			_editorGameObjectRemoveFunc(this, *it);
		#endif
		_gameObjects.erase(it);
	}
}
