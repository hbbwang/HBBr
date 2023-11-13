#pragma once
#include <memory>
#include <vector>
#include "Component.h"
#include "HString.h"
#include "Pass/PassType.h"
#include "Material.h"

class ModelComponent :public Component
{
	COMPONENT_DEFINE(ModelComponent)

public:

	void SetModelByRealPath(HString path);

	void SetModelByVirtualPath(HString path);

	void SetModel(HGUID guid);

	void SetModel(std::weak_ptr<class ModelData> model);

	virtual void GameObjectActiveChanged(bool objActive)override;

	virtual void ExecuteDestroy()override;

	virtual void Update()override;

	virtual void ClearPrimitves();

protected:

	//Component Property Reflection Add.
	HBBR_INLINE virtual void OnConstruction()override;

private:

	std::weak_ptr<class ModelData> _modelData;

	std::weak_ptr<class ModelData> _lastModelData;

	std::vector<ModelPrimitive*> _primitives;

	std::vector<std::weak_ptr<Material>>_materials;

};