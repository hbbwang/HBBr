#pragma once
#include "ContentManager.h"
class AssetObject
{
public:
	virtual ~AssetObject() = default;
	AssetInfoBase* _assetInfo = NULL;

};