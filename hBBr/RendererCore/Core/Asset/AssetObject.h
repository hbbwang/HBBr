#pragma once
#include "ContentManager.h"

class AssetObject
{
public:
	AssetObject()
	{
		SetToolTip();
	}

	virtual ~AssetObject() = default;

	AssetInfoBase* _assetInfo = nullptr;

	//编辑器ListWidget生成Item图标的时候触发...不要随意在其他(非编辑器)位置执行。
	//每个HString为单独一行
	//目前设定是，一般只有资产Load了才会执行
#if IS_EDITOR
	virtual void SetToolTip() {
		if (_assetInfo)
		{
			//_assetInfo->toolTips.push_back(HString::printf("资产类型:%s", GetAssetTypeString(_assetInfo->type)));
		}
	}
#endif

};