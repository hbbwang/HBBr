#pragma once
#include "ContentManager.h"
#include "Serializable.h"

class AssetObject :public Serializable
{
public:
	AssetObject()
	{
		bResident = false;
		SetToolTip();
	}

	virtual ~AssetObject() = default;

	//继承了AssetObject的资产类，必须重写以下函数：
	//		1.HBBR_API static std::weak_ptr<Material> LoadAsset(HGUID guid)
	//		2.HBBR_API virtual void SaveAsset(HString path) = 0;

	//
	HBBR_API virtual void SaveAsset(HString path) = 0;

	std::weak_ptr<AssetInfoBase> _assetInfo;

	//设置常驻资产,不会因为没有引用的情况下，被定时GC
	void SetResident(bool isResident)
	{
		bResident = isResident;
	}

	const bool GetResident()const {
		return bResident;
	}

	//编辑器ListWidget生成Item图标的时候触发...不要随意在其他(非编辑器)位置执行。
	//每个HString为单独一行
	//目前设定是，一般只有资产Load了才会执行
#if IS_EDITOR
	virtual void SetToolTip() {
		if (!_assetInfo.expired())
		{
		}
	}
#endif

protected:

	bool bResident = false;

};