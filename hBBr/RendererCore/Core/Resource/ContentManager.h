#pragma once
#include "Common.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include "RendererType.h"
#include "pugixml/pugixml.hpp"
#include "Resource/HGuid.h"
#include "HString.h"

//渲染器的资产名字在导入的过程中自动转换为GUID,而它的信息将会储存在Resource/Content/ContentReference.xml中。

//资产类型
enum class AssetType
{
	Unknow = 0,
	Model = 1,			//fbx
	Material = 2,		//mat
	Scene = 3,			//scene
	Texture2D = 4,		//tex2D
	TextureCube = 5,	//texCube
	Prefab = 6,			//frefab

	MaxNum = 32,
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

struct AssetInfoRefTemp
{
	HGUID guid;
	AssetType type;
};

class AssetInfoBase
{
public:
	AssetInfoBase() {}
	virtual ~AssetInfoBase() {}
	HGUID guid;
	AssetType type;
	HString name;
	HString suffix;
	HString relativePath;
	uint64_t byteSize;
	std::vector<AssetInfoBase*> refs;
	//用来暂时储存引用的guid和type,没有太多实际意义,通常是空的
	std::vector<AssetInfoRefTemp> refTemps;
	//
	bool bAssetLoad = false;

	inline const bool IsAssetLoad()const
	{
		return bAssetLoad;
	}
};

template<class T>
class AssetInfo : public AssetInfoBase
{
	std::unique_ptr<T> data = NULL;
public:
	AssetInfo():AssetInfoBase(){}
	virtual ~AssetInfo() { ReleaseData(); }
	inline T* GetData()const{
		return data.get();
	}
	inline void ReleaseData(){
		data.reset();
		data = NULL;
		bAssetLoad = false;
	}
	inline void SetData(std::unique_ptr<T> newData){
		data = std::move(newData);
		bAssetLoad = true;
	}
};

class ContentManager
{
	friend class VulkanApp;
public:
	~ContentManager();

	HBBR_API inline static ContentManager* Get() {
		if (!_ptr)
			_ptr.reset(new ContentManager());
		return _ptr.get(); 
	}
	
	/* 重载所有资产信息(只是加载引用信息,非资产本身) */
	HBBR_API void ReloadAllAssetInfos();

	/* 重载相关类型资产的信息(只是加载引用信息,非资产本身) */
	HBBR_API void ReloadAssetInfos(AssetType type);

	/* 更新所有资产引用关系 */
	HBBR_API void UpdateAllAssetReference();

	/* 根据AssetType更新资产的引用关系(Type) */
	HBBR_API void UpdateAssetReferenceByType(AssetType type);

	/* 更新单个资产的引用关系(GUID)&(Type) */
	HBBR_API void UpdateAssetReference(AssetType type , HGUID obj);

	/* 更新单个资产的引用关系(GUID),不指定Type,会全局检索,可能会比较慢 */
	HBBR_API void UpdateAssetReference(HGUID obj);

	/* 导入资产信息, 注意:该操作不会检查是否存在相同名字和路径的资产 */
	HBBR_API AssetInfoBase* ImportAssetInfo(AssetType type , HString sourcePath, HString contentPath);

	/* 删除资产 */
	HBBR_API void RemoveAssetInfo(HGUID obj, AssetType type = AssetType::Unknow);

	HBBR_API inline const std::unordered_map<HGUID, AssetInfoBase*>& GetAssets(AssetType type)const { return _assets[(uint32_t)type]; }

private:

	/* 更新单个资产的引用关系(info) */
	void UpdateAssetReference(AssetInfoBase* info);

	/* 重载单个资产的信息(只是加载引用信息,非资产本身) */
	void ReloadAssetInfo(AssetType type , pugi::xml_node& node);

	void Release();

	void ReleaseAssetsByType(AssetType type);

	void ReleaseAsset(AssetType type, HGUID obj);

	ContentManager();

	static std::unique_ptr<ContentManager> _ptr;

	//<HGUID,资产信息>//
	pugi::xml_document _contentRefConfig;

	std::vector<std::unordered_map<HGUID, AssetInfoBase*>>_assets;

	HString _configPath;
};
