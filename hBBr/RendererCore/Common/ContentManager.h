#pragma once
#include <memory>
#include "RendererType.h"
#include "pugixml/pugixml.hpp"
#include "Resource/HGuid.h"
#include <unordered_map>
#include <vector>
//资产类型
enum class AssetType
{
	Unknow = -1,
	Model = 0,
	Material = 1,
	Scene = 2,
};

struct AssetInfo
{
	HGUID guid;
	AssetType type;
	HString name;
	HString relativePath;
	std::vector<AssetInfo*> refs;
};

class ContentManager
{
public:

	inline static ContentManager* Get() { 
		if (!_ptr)
			_ptr = std::make_unique<ContentManager>(new ContentManager());
		return _ptr.get(); 
	}

	void ReloadAllAssetInfos();

	void UpdateReference(HGUID obj);

private:

	ContentManager();

	static std::unique_ptr<ContentManager> _ptr;

	//<HGUID,资产信息>//

	//models
	std::unordered_map<HGUID, AssetInfo>_ModelAssets;

	//materials
	std::unordered_map<HGUID, AssetInfo>_MaterialAssets;
};
