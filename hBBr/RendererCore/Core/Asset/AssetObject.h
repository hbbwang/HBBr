#pragma once
#include "ContentManager.h"
#include "Serializable.h"

class AssetObject :public Serializable
{
public:
	AssetObject()
	{
		SetToolTip();
	}

	virtual ~AssetObject() = default;

	std::weak_ptr<AssetInfoBase> _assetInfo;

	//编辑器ListWidget生成Item图标的时候触发...不要随意在其他(非编辑器)位置执行。
	//每个HString为单独一行
	//目前设定是，一般只有资产Load了才会执行
#if IS_EDITOR
	virtual void SetToolTip() {
		if (!_assetInfo.expired())
		{
			//_assetInfo->toolTips.push_back(HString::printf("资产类型:%s", GetAssetTypeString(_assetInfo->type)));
		}
	}
#endif

};