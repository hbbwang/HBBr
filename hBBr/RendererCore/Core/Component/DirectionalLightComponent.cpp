#include "DirectionalLightComponent.h"
#include "GameObject.h"
#include "Asset/World.h"
#include "VulkanRenderer.h"
#include "ConsoleDebug.h"
#include <map>
#include <memory>

#include "Pass/PassManager.h"

COMPONENT_IMPLEMENT(DirectionalLightComponent)

void DirectionalLightComponent::OnConstruction()
{
	Component::OnConstruction(); 
	AddProperty("Vector3", "Light Color", &_lightColor, false, "Default", 0, "", false);
	AddProperty("float", "Light Strength", &_lightStrength, false, "Default", 0, "", false);
	AddProperty("float", "Light Specular", &_lightSpecular, false, "Default", 0, "", false);
	AddProperty("bool", "Cast Shadow", &_bCastShadow, false, "Default", 0, "", false);

	_lightColor = glm::vec3(1,1,1);
	_lightStrength = 1.0;
	_lightSpecular = 1.0;
	_bCastShadow = true;
	_lightType = LightType_DirectionalLight;
	for (auto& i : _renderer->GetPassManagers())
	{
		i.second->BindLightingParameter(this);
	}
}

void DirectionalLightComponent::UpdateData()
{

}

void DirectionalLightComponent::Update()
{
}

void DirectionalLightComponent::ExecuteDestroy()
{
	for (auto& i : _renderer->GetPassManagers())
	{
		i.second->UnBindLightingParameter(this);
	}
}
