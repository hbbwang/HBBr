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

	HBBR_API void SetModelByRealPath(HString path);

	//path = virtual path (Content/...)
	HBBR_API void SetModelByAssetPath(HString virtualPath);

	//这个在未知assetInfo但是又清楚知道实际的资产guid的时候使用
	//写Hard code之类的就比较适合。
	HBBR_API void SetModel(HGUID guid);

	HBBR_API void SetModel(std::weak_ptr<class Model> model);

	virtual void GameObjectActiveChanged(bool objActive)override;

	virtual void ExecuteDestroy()override;

	virtual void Update()override;

	virtual void ClearPrimitves();

protected:

	//Component Property Reflection Add.
	HBBR_INLINE virtual void OnConstruction()override;

private:

	HGUID _modelGUID;

	HGUID _oldModelGUID;

	std::vector<HGUID> _materialGUIDs;

	std::vector<HGUID> _oldMaterialGUIDs;

	std::weak_ptr <Model> _modelCache;

	std::vector<ModelPrimitive*> _primitives;

	std::vector<std::weak_ptr<Material>>_materials;

};