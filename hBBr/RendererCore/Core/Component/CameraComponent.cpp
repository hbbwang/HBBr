#include "CameraComponent.h"
#include "GameObject.h"
#include "Asset/World.h"
#include "VulkanRenderer.h"
#include "HInput.h"
#include "ConsoleDebug.h"
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
	//Editor camera behavior.
	if (_bIsEditorCamera && !renderer->IsInGame())
	{
		static glm::vec2 lastMousePos;
		static glm::vec2 lockMousePos;
		glm::vec2 currentMousePos = GetMousePos();
		if (GetMouse(Button_Right))
		{					
			glm::vec2 mouseAxis = lastMousePos - currentMousePos;
			HInput::SetCursorPos(lockMousePos);
			lastMousePos = GetMousePos();
			
			float frameRate = (float)renderer->GetFrameRateS();
			worldRot.y -= mouseAxis.x * _editorMouseSpeed;
			worldRot.x -= mouseAxis.y * _editorMouseSpeed;
			trans->SetWorldRotation(worldRot);
			//
			if (GetKeyDown(Key_D) && GetKeyDown(Key_A))
			{
				ConsoleDebug::printf_endl_warning("GetKeyDown");
			}
			if (GetKeyDown(Key_W))
			{
				ConsoleDebug::printf_endl_warning("GetKeyDown : Key_W");
			}
			//if (GetMouseDown(Button_Left))
			//{
			//	ConsoleDebug::printf_endl_warning("GetMouseDown : Button_Left");
			//}
			//if (GetMouseUp(Button_Left))
			//{
			//	ConsoleDebug::printf_endl_warning("GetMouseUp : Button_Left");
			//}
			if (GetKey(Key_W))
			{
				worldPos += trans->GetForwardVector() * _editorMoveSpeed * frameRate;
			}
			if (GetKey(Key_S))
			{
				worldPos -= trans->GetForwardVector() * _editorMoveSpeed * frameRate;
			}
			if (GetKey(Key_A))
			{
				worldPos -= trans->GetRightVector() * _editorMoveSpeed * frameRate;
			}
			if (GetKey(Key_D))
			{
				worldPos += trans->GetRightVector() * _editorMoveSpeed * frameRate;
			}
			if (GetKey(Key_Q))
			{
				worldPos -= trans->GetUpVector() * _editorMoveSpeed * frameRate;
			}
			if (GetKey(Key_E))
			{
				worldPos += trans->GetUpVector() * _editorMoveSpeed * frameRate;
			}		
			trans->SetWorldLocation(worldPos);
		}
		//ConsoleDebug::print_endl(HString::FromVec2(mouseAxis));
		else
		{
			lastMousePos = currentMousePos;
			lockMousePos = currentMousePos;
		}
	}
	_cameraPos = worldPos;
	_cameraTarget = _cameraPos + trans->GetForwardVector();
	_cameraUp = trans->GetUpVector();
	//DirectX Left hand.
	_viewMatrix = glm::lookAtLH(_cameraPos, _cameraTarget, _cameraUp);
	_invViewMatrix = glm::inverse(_viewMatrix);
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
	Component::ExecuteDestroy();
}
