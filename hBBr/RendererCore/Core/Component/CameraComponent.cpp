#include "CameraComponent.h"
#include "GameObject.h"
#include "Asset/World.h"
#include "VulkanRenderer.h"
#include "HInput.h"
#include "ConsoleDebug.h"
#include "Pass/PassManager.h"
#include <map>
#include <memory>
CameraComponent::CameraComponent(GameObject* parent)
	:Component(parent)
{
	_cameraPos = glm::vec3(0.0f, 2.0f, -3.0f);
	_cameraTarget = glm::vec3(0, 0, 0);
	_cameraUp =glm::vec3(0, 1, 0);
	_nearClipPlane = 0.01f;
	_farClipPlane = 500.0f;
	_fov = 90.0f;
	_gameObject->GetWorld()->_cameras.push_back(this);
	if (_gameObject->GetWorld()->_mainCamera == nullptr)
	{
		_gameObject->GetWorld()->_mainCamera = this;
	}
	EnableKeyInput(true);
	EnableMouseInput(true);

	//为当前相机生成passes
	std::shared_ptr<PassManager>newPassManager;
	newPassManager.reset(new PassManager(_renderer));
	_renderer->_passManagers.emplace(this, newPassManager);

}

void CameraComponent::OverrideMainCamera()
{
	if (_gameObject->GetWorld()->_mainCamera != nullptr)
	{
		_gameObject->GetWorld()->_mainCamera->_bIsMainCamera = false;
	}
	_gameObject->GetWorld()->_mainCamera = this;
	_bIsMainCamera = true;
}

void CameraComponent::Update()
{
	const auto trans = _gameObject->GetTransform();
	auto worldPos = trans->GetWorldLocation();
	auto worldRot = trans->GetWorldRotation();
	auto renderer = _gameObject->GetWorld()->GetRenderer();

#if IS_EDITOR
	//Editor camera behavior.
	if (_bIsEditorCamera && !renderer->IsInGame())
	{
		glm::vec2 currentMousePos = HInput::GetMousePos();
		if (_cameraType == EditorCameraType::Free)
		{
			if (GetMouse(Button_Right))
			{
				glm::vec2 mouseAxis = lastMousePos - currentMousePos;
				SetMousePos(lockMousePos);
				lastMousePos = HInput::GetMousePos();

				double frameRate = VulkanApp::GetFrameRateS() * (double)_editorMoveSpeed;
				worldRot.y -= mouseAxis.x * _editorMouseSpeed;
				worldRot.x -= mouseAxis.y * _editorMouseSpeed;
				trans->SetWorldRotation(worldRot);
				if (GetKey(Key_W))
				{
					worldPos += trans->GetForwardVector() * (float)frameRate;
				}
				if (GetKey(Key_S))
				{
					worldPos -= trans->GetForwardVector() * (float)frameRate;
				}
				if (GetKey(Key_A))
				{
					worldPos -= trans->GetRightVector() * (float)frameRate;
				}
				if (GetKey(Key_D))
				{
					worldPos += trans->GetRightVector() * (float)frameRate;
				}
				if (GetKey(Key_Q))
				{
					worldPos -= trans->GetUpVector() * (float)frameRate;
				}
				if (GetKey(Key_E))
				{
					worldPos += trans->GetUpVector() * (float)frameRate;
				}
				trans->SetWorldLocation(worldPos);
			}
			//ConsoleDebug::print_endl(HString::FromVec2(mouseAxis));
			else
			{
				lastMousePos = currentMousePos;
				lockMousePos = currentMousePos;
			}
			_cameraPos = worldPos;
			_cameraTarget = _cameraPos + trans->GetForwardVector();
			_cameraUp = trans->GetUpVector();
			//DirectX Left hand.
			_viewMatrix = glm::lookAtLH(_cameraPos, _cameraTarget, _cameraUp);
			_invViewMatrix = glm::inverse(_viewMatrix);
		}
		else if (_cameraType == EditorCameraType::TargetRotation)
		{
			if (GetMouse(Button_Left) || GetMouse(Button_Right))
			{
				glm::vec2 mouseAxis = lastMousePos - currentMousePos;
				SetMousePos(lockMousePos);
				lastMousePos = HInput::GetMousePos();
				if (GetMouse(Button_Left))
				{
					worldRot.y -= mouseAxis.x * _editorMouseSpeed;
					worldRot.x -= mouseAxis.y * _editorMouseSpeed;
				}
				if (GetMouse(Button_Right))
				{
					_targetLength += mouseAxis.y * _editorMouseSpeed * 0.25f;
					_targetLength = glm::clamp(_targetLength , 0.0f , 200.0f);
				}
			}
			else
			{
				lastMousePos = currentMousePos;
				lockMousePos = currentMousePos;
			}

			trans->SetWorldRotation(worldRot);
			trans->SetWorldLocation(trans->GetForwardVector() * -_targetLength);
			_cameraPos = trans->GetWorldLocation();
			_cameraTarget =glm::vec3(0);
			_cameraUp = trans->GetUpVector();
			//DirectX Left hand.
			_viewMatrix = glm::lookAtLH(_cameraPos, _cameraTarget, _cameraUp);
			_invViewMatrix = glm::inverse(_viewMatrix);
		}
	}
#endif
}

void CameraComponent::ExecuteDestroy()
{
	const auto scene = _gameObject->GetWorld();
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
			scene->_mainCamera = nullptr;
		}
	}

	auto pit = _renderer->_passManagers.find(this);
	if (pit != _renderer->_passManagers.end())
	{
		_renderer->_passManagers.erase(pit);
	}

	Component::ExecuteDestroy();
}
