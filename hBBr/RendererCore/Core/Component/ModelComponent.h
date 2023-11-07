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

	void SetModel(HGUID guid);

	virtual void GameObjectActiveChanged(bool objActive)override;

	virtual void ExecuteDestroy()override;

	virtual void Update()override;

	virtual void ClearPrimitves();

private:

	HGUID _model;

	struct ModelData* _modelData = NULL ;

	std::vector<ModelPrimitive*> _primitives;

	std::vector<Material*>		_materials;

};