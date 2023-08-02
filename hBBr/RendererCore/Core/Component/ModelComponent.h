#pragma once
#include <memory>
#include <vector>
#include "Component.h"
#include "HString.h"
#include "Pass/PassType.h"
class ModelComponent :public Component
{
public:
	
	ModelComponent(class GameObject* parent);

	void SetModel(HString path);

private:

	HString _modelPath;

	struct ModelData* _modelData = NULL ;
	
	std::vector<Pass>	_pass;

	HString _vsShader;

	HString _psShader;
};