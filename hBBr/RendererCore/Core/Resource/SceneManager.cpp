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

//Test 
std::weak_ptr<GameObject> testObj;

void SceneManager::SceneInit(class VulkanRenderer* renderer)
{
	_renderer = renderer;

#if IS_EDITOR
	//create editor camera
	auto backCamera = new GameObject("EditorCamera", NULL, true);
	backCamera->GetTransform()->SetWorldLocation(glm::vec3(0, 2, -3.0));
	auto cameraComp = backCamera->AddComponent<CameraComponent>();
	cameraComp->OverrideMainCamera();
	_editorCamera = cameraComp;
	_editorCamera->_bIsEditorCamera = true;
#endif

	//Test
	auto test = new GameObject();
	auto modelComp0 = test->AddComponent<ModelComponent>();
	modelComp0->SetModel(FileSystem::GetResourceAbsPath() + "Content/Core/Baise/B7CF5F97-BFB9-4C36-893C-448B57776F69.FBX");
	test->SetObjectName("TestFbx_1_Combine");

	GameObject* cube = new GameObject();
	testObj = cube->_selfWeak;
	auto modelComp = cube->AddComponent<ModelComponent>();
	cube->GetTransform()->SetLocation(glm::vec3(0, 0.5f, 0));
	modelComp->SetModel(FileSystem::GetResourceAbsPath() + "Content/Core/Baise/B32B0C1E-D358-464F-8F0C-52F0DD0FAA05.FBX");
	cube->SetObjectName("TestFbx_Cube");
}
#include "HInput.h"
void SceneManager::SceneUpdate()
{
	//Test
	if (!testObj.expired() && testObj.lock()->GetTransform() && ( HInput::GetKey(KeyCode::F)))
	{
		testObj.lock()->GetTransform()->SetRotation(testObj.lock()->GetTransform()->GetRotation() + glm::vec3(0, _renderer->GetFrameRate() / 10.0f, 0));
	}

	//Destroy Objects
	const auto destroyCount = _gameObjectNeedDestroy.size();
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
