#pragma once
#include "ContentManager.h"

template<class T>
class Asset
{
public:
	std::weak_ptr<T> _assetPtr;
	?
};

class AssetObject
{
public:
	virtual ~AssetObject() = default;
	AssetInfoBase* _assetInfo = nullptr;

};