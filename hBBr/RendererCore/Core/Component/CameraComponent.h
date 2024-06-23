#pragma once

#include "Common.h"
#include "Component/Component.h"

enum class EditorCameraType
{
	Free,
	TargetRotation
};

class CameraComponent :public Component
{
	friend class World;
public:

	EditorCameraType _cameraType = EditorCameraType::Free;

	CameraComponent(class GameObject* parent);

	HBBR_INLINE glm::mat4 GetViewMatrix()const
	{
		return _viewMatrix;
	}

	HBBR_INLINE glm::mat4 GetInvViewMatrix()const
	{
		return _invViewMatrix;
	}

	HBBR_INLINE void SetFOV(float vaule)
	{
		_fov = vaule;
	}

	HBBR_INLINE float GetFOV()const
	{
		return _fov;
	}

	HBBR_INLINE void SetFarClipPlane(float vaule)
	{
		_farClipPlane = vaule;
	}

	HBBR_INLINE float GetFarClipPlane()const
	{
		return _farClipPlane;
	}

	HBBR_INLINE void SetNearClipPlane(float vaule)
	{
		_nearClipPlane = vaule;
	}

	HBBR_INLINE float GetNearClipPlane()const
	{
		return _nearClipPlane;
	}

	HBBR_INLINE glm::vec3 GetTargetPos()const
	{
		return _cameraTarget;
	}

	HBBR_INLINE bool IsMainCamera()const
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

	float _editorMoveSpeed = 1.0f;

	float _editorMouseSpeed = 0.25f;

	glm::vec2 lastMousePos;

	glm::vec2 lockMousePos;

	float _targetLength = 1.75f;
};