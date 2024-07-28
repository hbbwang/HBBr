#pragma once
#include "ContentManager.h"
#include "Serializable.h"

class AssetObject :public Serializable
{
public:
	AssetObject()
	{
		_bResident = false;
		_bSystemAsset = false;
		SetToolTip();
	}

	virtual ~AssetObject() = default;

	//继承了AssetObject的资产类，必须重写以下函数：
	//		1.HBBR_API static std::shared_ptr<Material> LoadAsset(HGUID guid)
	//		2.HBBR_API virtual void SaveAsset(HString path) = 0;

	//
	HBBR_API virtual void SaveAsset(HString path) = 0;

	std::weak_ptr<AssetInfoBase> _assetInfo;

	//设置常驻资产,不会因为没有引用的情况下，被定时GC
	void SetResident(bool isResident)
	{
		_bResident = isResident;
		_assetInfo.lock()->bResident = isResident;
	}

	const bool IsResident()const {
		return _bResident;
	}

	//设置系统资产,系统资产是永远不会被GC的
	void SetSystemAsset(bool isSystemAsset)
	{
		_bSystemAsset = isSystemAsset;
		_assetInfo.lock()->bSystemAsset = isSystemAsset;
	}

	const bool IsSystemAsset()const {
		return _bSystemAsset;
	}

	template<class T>
	static inline std::shared_ptr<T> Cast(std::shared_ptr<AssetObject> assetObject)
	{
		return std::static_pointer_cast<T>(assetObject);
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

	bool _bResident = false;

	bool _bSystemAsset = false;

};