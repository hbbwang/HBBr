#pragma once
#include "Common.h"

class GameObject;
class Transform
{
public:
	Transform(GameObject* parent, glm::vec3 pos = glm::vec3(0,0,0), glm::vec3 eulerAge = glm::vec3(0,0,0), glm::vec3 scale3D = glm::vec3(1,1,1));
	virtual ~Transform();

	HBBR_API HBBR_INLINE glm::vec3 GetForwardVector()const{return forward;}

	HBBR_API HBBR_INLINE glm::vec3 GetRightVector()const{return right;}

	HBBR_API HBBR_INLINE glm::vec3 GetUpVector()const{return up;}

	HBBR_API HBBR_INLINE glm::vec3 GetWorldLocation()const {return worldLocation;}

	HBBR_API HBBR_INLINE glm::vec3 GetWorldRotation()const { return worldEulerAngle; }

	HBBR_API HBBR_INLINE glm::vec3 GetLocation()const { return location; }

	HBBR_API HBBR_INLINE glm::vec3 GetRotation()const { return eulerAngle; }

	HBBR_API HBBR_INLINE glm::vec3 GetScale3D()const { return scale3D; }

	HBBR_API HBBR_INLINE glm::mat4 GetWorldMatrix()const { return worldMatrix; }

	//Set object scale at local 3D space.
	HBBR_API HBBR_INLINE  void SetScale3D(glm::vec3 newSize)
	{
		FSetScale3D(newSize);
	}

	//Set object world location ( no attachment )
	HBBR_API HBBR_INLINE void SetWorldLocation(glm::vec3 newLocation)
	{
		FSetWorldLocation(newLocation);
	}

	//Set object world rotation ( no attachment )
	HBBR_API HBBR_INLINE void SetWorldRotation(glm::vec3 newAngle)
	{
		FSetWorldRotation(newAngle);
	}

	//Set object location ( with attachment )
	HBBR_API HBBR_INLINE void SetLocation(glm::vec3 newLocation)
	{
		FSetLocation(newLocation);
	}

	//Set object rotation ( with attachment )
	HBBR_API HBBR_INLINE void SetRotation(glm::vec3 newAngle)
	{
		FSetRotation(newAngle);
	}

	HBBR_API void SetWorldMatrix(glm::mat4 newWorldMat);

	HBBR_API HBBR_INLINE void SetLocationAndRotation(glm::vec3 newLocation, glm::vec3 newAngle)
	{
		FSetRotation(newAngle);
		FSetLocation(newLocation);
	}

	HBBR_API HBBR_INLINE void SetWorldLocationAndRotation(glm::vec3 newLocation, glm::vec3 newAngle)
	{
		FSetWorldRotation(newAngle);
		FSetWorldLocation(newLocation);
	}

	HBBR_API void UpdateChildrenTransform();

	void UpdateChildrenLocation();

	void UpdateChildrenRotation();

	void UpdateChildrenScale3D();

	void ResetTransformForAttachment();

	/* 更新Transform */
	HBBR_API virtual void UpdateTransformByVariable();

	/*逻辑更新*/
	virtual void Update();

	HBBR_INLINE bool NeedUpdateUb()const { return _bNeedUpdateUniformBuffer; }

private:

	glm::vec3 scale3D = glm::vec3(1, 1, 1);

	//the location same as worldLocation when the object is not having parent.
	glm::vec3 location = glm::vec3(0, 0, 0);

	//the eulerAngle same as worldEulerAngle when the object is not having parent.
	glm::vec3 eulerAngle = glm::vec3(0, 0, 0);

	glm::vec3 forward = glm::vec3(0, 0, 0);

	glm::vec3 right = glm::vec3(0, 0, 0);

	glm::vec3 up = glm::vec3(0, 0, 0);

	glm::vec3 worldLocation = glm::vec3(0, 0, 0);

	glm::vec3 worldEulerAngle = glm::vec3(0, 0, 0);

	glm::mat4 worldMatrix;
	
	class GameObject* _gamebject = NULL;

	HBBR_API void FSetWorldLocation(glm::vec3 newWorldLocation, bool bAffectChildren = true);
	HBBR_API void FSetWorldRotation(glm::vec3 newAngle, bool bAffectChildren = true);
	HBBR_API void FSetLocalRotation(glm::vec3 newAngle);
	HBBR_API void FSetScale3D(glm::vec3 newSize, bool bAffectChildren = true);
	HBBR_API void FSetLocation(glm::vec3 newLocation);
	HBBR_API void FSetRotation(glm::vec3 newRotation);

	bool _bNeedUpdateUniformBuffer;

	uint8_t UpdateStateCount = 0 ;
};

