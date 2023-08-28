#pragma once

#include "Common.h"
#include "Component/Component.h"

class CameraComponent :public Component
{
	friend class SceneManager;
public:
	
	CameraComponent(class GameObject* parent);

	__forceinline glm::mat4 GetViewMatrix()const
	{
		return _viewMatrix;
	}

	__forceinline glm::mat4 GetInvViewMatrix()const
	{
		return _invViewMatrix;
	}

	__forceinline void SetFOV(float vaule)
	{
		_fov = vaule;
	}

	__forceinline float GetFOV()const
	{
		return _fov;
	}

	__forceinline void SetFarClipPlane(float vaule)
	{
		_farClipPlane = vaule;
	}

	__forceinline float GetFarClipPlane()const
	{
		return _farClipPlane;
	}

	__forceinline void SetNearClipPlane(float vaule)
	{
		_nearClipPlane = vaule;
	}

	__forceinline float GetNearClipPlane()const
	{
		return _nearClipPlane;
	}

	__forceinline glm::vec3 GetTargetPos()const
	{
		return _cameraTarget;
	}

	__forceinline bool IsMainCamera()const
	{
		return _bIsMainCamera;
	}

	void OverrideMainCamera();

	void Update()override;

private:

	void ExecuteDestroy()override;

	bool _bIsMainCamera = false;

	glm::vec3 _cameraPos;

	glm::vec3 _cameraTarget;

	glm::vec3 _cameraUp;

	float _nearClipPlane;

	float _farClipPlane;

	float _fov;

	glm::mat4 _viewMatrix;

	glm::mat4 _invViewMatrix;

	bool _bIsEditorCamera = false;
};