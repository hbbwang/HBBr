#pragma once
#include "Common.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include "RendererType.h"
#include "pugixml/pugixml.hpp"
#include "Asset/HGuid.h"
#include "HString.h"
#include "Asset/HGuid.h"

//资产类型
enum class AssetType : uint32_t
{
	Unknow = 0,
	Model = 1,			//.fbx
	Material = 2,		//.mat
	Texture2D = 3,		//.tex2D

	MaxNum = 4,
};


inline static AssetType GetAssetTypeBySuffix(HString suffix)
{
	if (suffix.IsSame("fbx", false))
	{
		return AssetType::Model;
	}
	else if (suffix.IsSame("mat", false))
	{
		return AssetType::Material;
	}
	else if(suffix.IsSame("dds", false))
	{
		return AssetType::Texture2D;
	}
	return AssetType::Unknow;
}

inline static HString GetAssetTypeString(AssetType type)
{
	switch (type)
	{
	case AssetType::Model:return "Model";
	case AssetType::Material:return "Material";
	case AssetType::Texture2D:return "Texture2D";
	case AssetType::Unknow:
	case AssetType::MaxNum:	return "Unknow";
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
	//虚拟名称,非实际文件名
	HString displayName;
	HString suffix;
	//资产所在的真实绝对 [文件] 路径(D:/xxx/xxx/.../abc.fbx)，带完整文件名字和后缀
	HString absFilePath;
	//资产所在的真实绝对 [目录] 路径(D:/xxx/xxx/...)
	HString absPath;
	//资产所在的相对 [文件] 路径(Asset/..../abc.fbx),带完整文件名字和后缀
	HString assetFilePath;
	//资产所在的相对 [目录] 路径(Asset/....)
	HString assetPath;
	//资产的虚拟路径(Content/xxx/...),非实际路径
	HString virtualPath;
	//资产的虚拟文件路径(Content/xxx/...abc.fbx),非实际路径,带名字和后缀
	HString virtualFilePath;
	//资产所在的仓库名字,不带.repository后缀
	HString repository;
	std::vector<std::weak_ptr<AssetInfoBase>> refs;
	//用来暂时储存引用的guid和type,没有太多实际意义,通常是空的
	std::vector<AssetInfoRefTemp> refTemps;
	//
	bool bAssetLoad = false;
	virtual std::weak_ptr<class AssetObject> GetAssetData()const { return std::weak_ptr<class AssetObject>(); }
	virtual void ReleaseData() {}
	inline const bool IsAssetLoad()const
	{
		return bAssetLoad;
	}
};

template<class T>
class AssetInfo : public AssetInfoBase
{
	std::shared_ptr<T> data = nullptr;
public:
	AssetInfo():AssetInfoBase(){}
	virtual ~AssetInfo() { ReleaseData(); }
	inline std::weak_ptr<T> GetData()const{
		if (data)
		{
			return data;
		}
		return T::LoadAsset(this->guid);
	}
	inline std::weak_ptr<class AssetObject> GetAssetData()const override {
		if (data)
		{
			return data;
		}
		return T::LoadAsset(this->guid);
	}
	inline void ReleaseData() override{
		data.reset();
		data = nullptr;
		bAssetLoad = false;
	}
	inline void SetData(std::shared_ptr<T> newData){
		data = std::move(newData);
		bAssetLoad = true;
	}
};

struct AssetSaveType
{
	HString metaAssetPath;
	HGUID guid;
	AssetType type;
	size_t byteSize;
	std::vector<std::weak_ptr<AssetInfoBase>> refs;
};

struct Repository
{
	HString Name;
	std::
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

	/* 更新所有资产引用关系 */
	HBBR_API void UpdateAllAssetReference();

	/* 根据AssetType更新资产的引用关系(Type) */
	HBBR_API void UpdateAssetReferenceByType(AssetType type);

	/* 更新单个资产的引用关系(GUID)&(Type) */
	HBBR_API void UpdateAssetReference(AssetType type , HGUID obj);

	/* 更新单个资产的引用关系(GUID),不指定Type,会全局检索,可能会比较慢 */
	HBBR_API void UpdateAssetReference(HGUID obj);

	HBBR_API inline const std::unordered_map<HGUID, std::shared_ptr<AssetInfoBase>>& GetAssets(AssetType type)const { return _assets[(uint32_t)type]; }

	HBBR_API std::weak_ptr<AssetInfoBase> GetAssetInfo(HGUID guid, AssetType type = AssetType::Unknow)const;

	HBBR_API std::weak_ptr<AssetInfoBase> GetAssetInfo(HGUID guid,HString repositoryName)const;

	/* 根据内容浏览器显示的文件名(文件的虚拟路径)称查找GUID,不推荐使用 */
	HBBR_API std::weak_ptr<AssetInfoBase> GetAssetInfo(HString virtualFilePath , AssetType type = AssetType::Unknow)const;

	/* 重载仓库 */
	HBBR_API void ReloadRepository(HString repositoryName);

	template<class T>
	HBBR_INLINE std::weak_ptr<T> GetAsset(HGUID guid , AssetType type = AssetType::Unknow)
	{
		auto assetInfo = GetAssetInfo(guid , type);
		if (assetInfo.expired())
		{
			return std::weak_ptr<T>();
		}
		auto asset = std::static_pointer_cast<AssetInfo<T>>(assetInfo.lock());
		return asset->GetData();
	}

	//通过已知类型和guid加载资产
	template<class T>
	HBBR_INLINE std::weak_ptr<class AssetObject> LoadAsset(HGUID guid)
	{
		return T::LoadAsset(guid);
	}

	template<class T>
	HBBR_INLINE std::weak_ptr<T> LoadAsset(HString assetPath)
	{
		if (!assetPath.IsEmpty())
		{
			return T::LoadAsset(GetAssetGUID(assetPath));
		}
		return  std::weak_ptr<T>();
	}

private:

	/* 更新单个资产的引用关系(info) */
	void UpdateAssetReference(std::weak_ptr<AssetInfoBase> info);

	void Release();

	ContentManager();

	static std::unique_ptr<ContentManager> _ptr;

	//根据类型储存对象
	std::vector<std::unordered_map<HGUID, std::shared_ptr<AssetInfoBase>>>_assets;
	//根据仓库储存对象
	std::unordered_map<HString, std::unordered_map<HGUID, std::shared_ptr<AssetInfoBase>>> _assets_repos;

};
