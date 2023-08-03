#include "Transform.h"
#include "GameObject.h"
#include "glm/glm.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
Transform::Transform(GameObject* parent, glm::vec3 pos , glm::vec3 eulerAge, glm::vec3 scale3D)
{
	_gamebject = parent;
	worldMatrix = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	right = glm::vec3(1.0f,0.0f,0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	forward = glm::vec3(0.0f, 0.0f, 1.0f);

	SetLocationAndRotation(pos, eulerAge);
}

Transform::~Transform()
{
}

void Transform::Update()
{
	//if (localLoc != location)
	//{
	//	SetLocation(location);
	//	localLoc = location;
	//}
	//else if (localRot != eulerAngle)
	//{
	//	SetRotation(eulerAngle);
	//	localRot = eulerAngle;
	//}
	//else if (localscal != scale3D)
	//{
	//	SetScale3D(scale3D);
	//	localscal = scale3D;
	//}
}

void Transform::SetWorldMatrix(glm::mat4 newWorldMat)
{
	if (worldMatrix != newWorldMat)
	{
		//pos
		FSetWorldLocation(glm::vec3(newWorldMat[3].x, newWorldMat[3].y, newWorldMat[3].z), false);
		//rot
		float x, y, z;
		glm::vec4 pitch = normalize(newWorldMat[0]);
		glm::vec4 yaw = normalize(newWorldMat[1]);
		glm::vec4 roll = normalize(newWorldMat[2]);
		glm::extractEulerAngleYXZ(glm::mat4(pitch, yaw, roll, glm::vec4(0, 0, 0, 1)), y, x, z);
		worldEulerAngle = glm::vec3(glm::degrees(x), glm::degrees(y), glm::degrees(z));
		FSetWorldRotation(worldEulerAngle, false);
		//scale
		x = glm::length(newWorldMat[0]);
		y = glm::length(newWorldMat[1]);
		z = glm::length(newWorldMat[2]);
		FSetScale3D(glm::vec3(x, y, z), false);
		UpdateChildrenTransform();
		worldMatrix = newWorldMat;
	}
}

void Transform::UpdateChildrenTransform()
{
	for (auto i : _gamebject->GetChildren())
	{
		i->GetTransform()->UpdateTransformByVariable();
	}
}

void Transform::UpdateChildrenLocation()
{
	for (auto i : _gamebject->GetChildren())
	{
		i->GetTransform()->FSetLocation(i->GetTransform()->location);
	}
}

void Transform::UpdateChildrenRotation()
{
	for (auto i : _gamebject->GetChildren())
	{
		i->GetTransform()->FSetRotation(i->GetTransform()->eulerAngle);
	}
}

void Transform::UpdateChildrenScale3D()
{
	for (auto i : _gamebject->GetChildren())
	{
		i->GetTransform()->FSetScale3D(i->GetTransform()->scale3D);
	}
}

void Transform::UpdateTransformByVariable()
{
	FSetLocation(location);
	FSetRotation(eulerAngle);
	FSetScale3D(scale3D);
}

void Transform::FSetWorldLocation(glm::vec3 newWorldLocation, bool bAffectChildren)
{
	worldLocation = newWorldLocation;
	glm::vec4 w = glm::vec4(worldLocation.x, worldLocation.y, worldLocation.z, 1.0);
	worldMatrix[3] = w;

	//Get local location
	if (_gamebject->GetParent() == NULL)
	{
		location = worldLocation;
	}
	else
	{
		//world to local
		//inverseTranspose �Ǹ���һ�����Direction�õ�
		location = glm::inverse(_gamebject->GetParent()->GetTransform()->worldMatrix) * w;
	}

	if (bAffectChildren)
		UpdateChildrenLocation();
}

void Transform::FSetWorldRotation(glm::vec3 newAngle, bool bAffectChildren)
{
	worldEulerAngle = newAngle;
	worldEulerAngle.x = std::fmodf(worldEulerAngle.x, 360.0);
	worldEulerAngle.y = std::fmodf(worldEulerAngle.y, 360.0);
	worldEulerAngle.z = std::fmodf(worldEulerAngle.z, 360.0);

	glm::vec3 _eulerAngle = glm::vec3(
		glm::radians(worldEulerAngle.x),
		glm::radians(worldEulerAngle.y),
		glm::radians(worldEulerAngle.z)
	);

	glm::mat4 rot = glm::yawPitchRoll(_eulerAngle.y, _eulerAngle.x, _eulerAngle.z);

	//
	right =		glm::normalize(glm::vec3(rot[0].x, rot[0].y, rot[0].z));
	up =		glm::normalize(glm::vec3(rot[1].x, rot[1].y, rot[1].z));
	forward =	glm::normalize(glm::vec3(rot[2].x, rot[2].y, rot[2].z));
	//
	worldMatrix[0] = glm::vec4(right.x, right.y, right.z, 0.0);
	worldMatrix[1] = glm::vec4(up.x, up.y, up.z, 0.0);
	worldMatrix[2] = glm::vec4(forward.x, forward.y, forward.z, 0.0);

	//Update local eulerAngle
	if (_gamebject->GetParent() == NULL)
	{
		eulerAngle = worldEulerAngle;
	}
	else
	{
		auto parentSpace = glm::inverse(_gamebject->GetParent()->GetTransform()->worldMatrix) * worldMatrix;
		float x, y, z;
		glm::extractEulerAngleYXZ(parentSpace, y, x, z);
		eulerAngle = glm::vec3(
			glm::degrees(x),
			glm::degrees(y),
			glm::degrees(z)
		);
	}

	FSetScale3D(scale3D);

	if(bAffectChildren)
		UpdateChildrenTransform();
}

void Transform::FSetScale3D(glm::vec3 newSize, bool bAffectChildren)
{
	scale3D = newSize;
	if (_gamebject->GetParent())
	{
		FSetLocalRotation(eulerAngle);
		return;
	}

	glm::vec3 f = forward, r = right, u = up;

	r = r * scale3D.x;
	u = u * scale3D.y;
	f = f * scale3D.z;

	worldMatrix[0] = glm::vec4(r.x, r.y, r.z, worldMatrix[0].w);
	worldMatrix[1] = glm::vec4(u.x, u.y, u.z, worldMatrix[1].w);
	worldMatrix[2] = glm::vec4(f.x, f.y, f.z, worldMatrix[2].w);

	if (bAffectChildren)
		UpdateChildrenScale3D();
}

void Transform::FSetLocalRotation(glm::vec3 newAngle)
{
	glm::vec3 _eulerAngle = newAngle;
	_eulerAngle.x = std::fmodf(_eulerAngle.x, 360.0);
	_eulerAngle.y = std::fmodf(_eulerAngle.y, 360.0);
	_eulerAngle.z = std::fmodf(_eulerAngle.z, 360.0);

	_eulerAngle = glm::vec3(
		glm::radians(_eulerAngle.x),
		glm::radians(_eulerAngle.y),
		glm::radians(_eulerAngle.z)
	);

	//����Euler Angle��ȡ����ת����
	glm::mat4 rot = glm::yawPitchRoll(_eulerAngle.y, _eulerAngle.x, _eulerAngle.z);

	rot[0] = rot[0] * scale3D.x;
	rot[1] = rot[1] * scale3D.y;
	rot[2] = rot[2] * scale3D.z;

	if (_gamebject->GetParent())
	{
		//�������Ǿֲ��ռ䣬�ҳ�Parent���������ת������������
		rot = _gamebject->GetParent()->GetTransform()->worldMatrix * rot;
	}
	//
	right =		(glm::vec3(rot[0].x, rot[0].y, rot[0].z));
	up =		(glm::vec3(rot[1].x, rot[1].y, rot[1].z));
	forward =	(glm::vec3(rot[2].x, rot[2].y, rot[2].z));
	//
	worldMatrix[0] = glm::vec4(right.x, right.y, right.z, 0.0);
	worldMatrix[1] = glm::vec4(up.x, up.y, up.z, 0.0);
	worldMatrix[2] = glm::vec4(forward.x, forward.y, forward.z, 0.0);

	right = glm::normalize(right);
	up = glm::normalize(up);
	forward = glm::normalize(forward);

	//Update local eulerAngle
	if (_gamebject->GetParent() == NULL)
	{
		worldEulerAngle = newAngle;
		eulerAngle = worldEulerAngle;
	}
	else
	{
		eulerAngle = newAngle;
		float x, y, z;
		glm::extractEulerAngleYXZ(worldMatrix, y, x, z);
		worldEulerAngle = glm::vec3(
			glm::degrees(x),
			glm::degrees(y),
			glm::degrees(z)
		);
	}
	UpdateChildrenTransform();
}

void Transform::FSetLocation(glm::vec3 newLocation)
{
	location = newLocation;
	if (_gamebject->GetParent())
	{
		Transform* parent = _gamebject->GetParent()->GetTransform();
		newLocation = parent->worldMatrix * glm::vec4(newLocation, 1.0);
		FSetWorldLocation(newLocation);
	}
	else
	{
		FSetWorldLocation(newLocation);
	}
}

void Transform::FSetRotation(glm::vec3 newRotation)
{
	eulerAngle = newRotation;
	if (_gamebject->GetParent())
	{
		FSetLocalRotation(newRotation);
	}
	else
	{
		FSetWorldRotation(newRotation);
	}
}