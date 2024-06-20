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

	HBBR_API virtual void SetModel(std::weak_ptr<class Model> model);

	virtual void GameObjectActiveChanged(bool objActive)override;

	virtual void ExecuteDestroy()override;

	virtual void Update()override;

	virtual void ClearPrimitves();

protected:
	//不是每帧执行,一般用于编辑器的对象数据刷新
	virtual void UpdateData()override;

	//Component Property Reflection Add.
	HBBR_INLINE virtual void OnConstruction()override;

	void UpdateMaterial();

private:

	AssetRef  _modelPath;

	std::vector<AssetRef> _materialPath;

	std::vector<ModelPrimitive*> _primitives;

};