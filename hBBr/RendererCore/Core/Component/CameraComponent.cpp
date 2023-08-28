#include "CameraComponent.h"
#include "GameObject.h"
#include "Resource/SceneManager.h"

CameraComponent::CameraComponent(GameObject* parent)
	:Component(parent)
{
	_cameraPos = glm::vec3(0.0f, 2.0f, -3.0f);
	_cameraTarget = glm::vec3(0, 0, 0);
	_cameraUp =glm::vec3(0, 1, 0);
	_nearClipPlane = 0.01f;
	_farClipPlane = 500.0f;
	_fov = 90.0f;
	_gameObject->GetScene()->_cameras.push_back(this);
	if (_gameObject->GetScene()->_mainCamera == NULL)
	{
		_gameObject->GetScene()->_mainCamera = this;
	}
}

void CameraComponent::OverrideMainCamera()
{
	if (_gameObject->GetScene()->_mainCamera != NULL)
	{
		_gameObject->GetScene()->_mainCamera->_bIsMainCamera = false;
	}
	_gameObject->GetScene()->_mainCamera = this;
	_bIsMainCamera = true;
}

void CameraComponent::Update()
{
	//Editor camera behavior.
	if (_bIsEditorCamera)
	{

	}
	const auto trans = _gameObject->GetTransform();
	_cameraPos = trans->GetWorldLocation();
	_cameraTarget = trans->GetForwardVector() - _cameraPos;
	_cameraUp = trans->GetUpVector();
	_viewMatrix = glm::lookAt(_cameraPos, _cameraTarget, _cameraUp);
	_invViewMatrix = glm::inverse(_viewMatrix);
}

void CameraComponent::ExecuteDestroy()
{
	const auto scene = _gameObject->GetScene();
	auto it = std::remove(scene->_cameras.begin(), scene->_cameras.end(), this);
	if (it != scene->_cameras.end())
	{
		scene->_cameras.erase(it);
	}
	if (scene->_mainCamera == this)
	{
		if (scene->_cameras.size() > 0)
		{
			scene->_mainCamera = scene->_cameras[scene->_cameras.size() - 1];
		}
		else
		{
			scene->_mainCamera = NULL;
		}
	}
	Component::ExecuteDestroy();
}
