#pragma once
#include "Common.h"

class GameObject;
class Transform
{
public:
	Transform(GameObject* parent, glm::vec3 pos = glm::vec3(0,0,0), glm::vec3 eulerAge = glm::vec3(0,0,0), glm::vec3 scale3D = glm::vec3(1,1,1));
	~Transform();

	HBBR_API __forceinline glm::vec3 GetForwardVector()const{return forward;}

	HBBR_API __forceinline glm::vec3 GetRightVector()const{return right;}

	HBBR_API __forceinline glm::vec3 GetUpVector()const{return up;}

	HBBR_API __forceinline glm::vec3 GetWorldLocation()const {return worldLocation;}

	HBBR_API __forceinline glm::vec3 GetWorldRotation()const { return worldEulerAngle; }

	HBBR_API __forceinline glm::vec3& GetLocation() { return location; }

	HBBR_API __forceinline glm::vec3& GetRotation() { return eulerAngle; }

	HBBR_API __forceinline glm::vec3& GetScale3D() { return scale3D; }

	HBBR_API __forceinline glm::mat4& GetWorldMatrix() { return worldMatrix; }

	//Set object scale at local 3D space.
	HBBR_API __forceinline  void SetScale3D(glm::vec3 newSize)
	{
		FSetScale3D(newSize);
	}

	//Set object world location ( no attachment )
	HBBR_API __forceinline void SetWorldLocation(glm::vec3 newLocation)
	{
		FSetWorldLocation(newLocation);
	}

	//Set object world rotation ( no attachment )
	HBBR_API __forceinline void SetWorldRotation(glm::vec3 newAngle)
	{
		FSetWorldRotation(newAngle);
	}

	//Set object location ( with attachment )
	HBBR_API __forceinline void SetLocation(glm::vec3 newLocation)
	{
		FSetLocation(newLocation);
	}

	//Set object rotation ( with attachment )
	HBBR_API __forceinline void SetRotation(glm::vec3 newAngle)
	{
		FSetRotation(newAngle);
	}

	HBBR_API void SetWorldMatrix(glm::mat4 newWorldMat);

	HBBR_API __forceinline void SetLocationAndRotation(glm::vec3 newLocation, glm::vec3 newAngle)
	{
		FSetRotation(newAngle);
		FSetLocation(newLocation);
	}

	HBBR_API __forceinline void SetWorldLocationAndRotation(glm::vec3 newLocation, glm::vec3 newAngle)
	{
		FSetWorldRotation(newAngle);
		FSetWorldLocation(newLocation);
	}

	HBBR_API void UpdateChildrenTransform();

	void UpdateChildrenLocation();

	void UpdateChildrenRotation();

	void UpdateChildrenScale3D();

	/* 更新Transform */
	HBBR_API virtual void UpdateTransformByVariable();

	/*逻辑更新*/
	virtual void Update();

	__forceinline bool NeedUpdateUb()const { return _bNeedUpdateUniformBuffer; }

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

	glm::vec3 localLoc = glm::vec3(0, 0, 0);
	glm::vec3 localRot = glm::vec3(0, 0, 0);
	glm::vec3 localscal = glm::vec3(0, 0, 0);

	bool _bNeedUpdateUniformBuffer;

	uint8_t UpdateStateCount = 0 ;
};

