#pragma once

#include "Common.h"
#include "Component/Component.h"
#include "Pass/PassType.h"
#include "Pass/UniformBuffer.h"

class DirectionalLightComponent :public Component
{
	COMPONENT_DEFINE(DirectionalLightComponent)

protected:
	//Component Property Reflection Add.
	virtual void OnConstruction()override;
	//不是每帧执行,一般用于编辑器的对象数据刷新
	virtual void UpdateData()override;

public: 
	virtual void Update()override;

	virtual LightType GetLightType()const {
		return _lightType;
	}

	virtual glm::vec3 GetLightColor()const {
		return _lightColor;
	}
	virtual void SetLightColor(glm::vec3 newColor) {
		_lightColor = newColor;
	}

	virtual float GetLightIntensity()const {
		return _lightStrength;
	}
	virtual void SetLightIntensity(float newIntensity) {
		_lightStrength = newIntensity;
	}

	virtual bool IsCastShadow()const {
		return _bCastShadow;
	}
	virtual void SetCastShadow(bool bCastShadow) {
		_bCastShadow = bCastShadow;
	}

	virtual float GetLightSpecular()const {
		return _lightSpecular;
	}
	virtual void SetLightSpecular(float specular) {
		_lightSpecular = specular;
	}

protected:
	virtual void ExecuteDestroy()override;
private:
	glm::vec3	_lightColor;
	float		_lightStrength;
	float		_lightSpecular;
	LightType	_lightType;
	bool		_bCastShadow;
};