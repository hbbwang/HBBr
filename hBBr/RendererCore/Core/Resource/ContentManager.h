#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include "RendererType.h"
#include "pugixml/pugixml.hpp"
#include "Resource/HGuid.h"
#include "HString.h"

//资产类型
enum class AssetType
{
	Unknow = -1,
	Model = 0,			//fbx
	Material = 1,		//mat
	Scene = 2,			//scene
	Texture2D = 3,		//tex2D
	TextureCube = 4,	//texCube
	Prefab = 5,			//frefab
};

inline static HString GetAssetTypeString(AssetType type)
{
	switch (type)
	{
	case AssetType::Model:return "Model";
	case AssetType::Material:return "Material";
	case AssetType::Scene:return "Scene";
	case AssetType::Texture2D:return "Texture2D";
	case AssetType::TextureCube:return "TextureCube";
	case AssetType::Prefab:return "Prefab";
	}
	return "Unknow";
}


class AssetInfoBase
{
public:
	AssetInfoBase() {}
	virtual ~AssetInfoBase() {}
	HGUID guid;
	AssetType type;
	HString name;
	HString relativePath;
	std::vector<AssetInfoBase*> refs;
};

template<typename T>
class AssetInfo : public AssetInfoBase
{
	bool bAssetLoad = false;
	std::unique_ptr<T> data = NULL;

public:
	AssetInfo():AssetInfoBase(){}
	virtual ~AssetInfo() { ReleaseData(); }
	inline T* GetData()const 
	{
		return data.get();
	}

	inline const bool IsAssetLoad()const
	{
		return bAssetLoad;
	}

	inline void ReleaseData()
	{
		data.reset();
		data = NULL;
		bAssetLoad = false;
	}

	inline void SetData(std::unique_ptr<T> newData)
	{
		data = std::move(newData);
		bAssetLoad = true;
	}

};

class ContentManager
{
	friend class VulkanApp;
public:
	~ContentManager();

	inline static ContentManager* Get() { 
		if (!_ptr)
			_ptr.reset(new ContentManager());
		return _ptr.get(); 
	}

	void ReloadAllAssetInfos();

	void ReloadAssetInfos(AssetType type);

	void UpdateReference(HGUID obj);

	inline const std::unordered_map<HGUID, AssetInfoBase*>& GetModelAssets()const { return _ModelAssets; }

	inline const std::unordered_map<HGUID, AssetInfoBase*>& GetMaterialAssets()const { return _MaterialAssets; }

private:

	void Release();

	void ReleaseAssetsByType(AssetType type);

	ContentManager();

	static std::unique_ptr<ContentManager> _ptr;

	//<HGUID,资产信息>//
	pugi::xml_document _contentRefConfig;

	//models
	std::unordered_map<HGUID, AssetInfoBase*>_ModelAssets;

	//materials
	std::unordered_map<HGUID, AssetInfoBase*>_MaterialAssets;
};
