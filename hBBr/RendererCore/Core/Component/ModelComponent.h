#pragma once
#include <memory>
#include <vector>
#include "Component.h"
#include "HString.h"
#include "Pass/PassType.h"
#include "Material.h"

class ModelComponent :public Component
{
public:
	
	ModelComponent(class GameObject* parent);

	void SetModel(HString path);

	virtual void SetActive(bool newActive)override;

private:

	HString _modelPath;

	struct ModelData* _modelData = NULL ;

	std::vector<ModelPrimitive> _primitive;

	std::vector<Material*>		_materials;

};