#pragma once
#include "Component.h"
#include "HString.h"
#include <memory>
class ModelComponent :public Component
{
public:
	
	ModelComponent(class GameObject* parent);

	void SetModel(HString path);

private:

	HString _modelPath;

	struct ModelData* _modelData = NULL ;

};