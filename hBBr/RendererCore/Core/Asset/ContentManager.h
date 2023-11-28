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
//渲染器的资产名字在导入的过程中自动转换为GUID,而它的信息将会储存在Asset/Content/ContentReference.xml中。

//资产类型
enum class AssetType
{
	Unknow = 0,
	Model = 1,			//.fbx
	Material = 2,		//.mat
	World = 3,			//.world
	Texture2D = 4,		//.tex2D
	TextureCube = 5,	//.texCube
	Prefab = 6,			//.frefab
	Level = 7,				//.level
	MaxNum = 32,
};

inline static HString GetAssetTypeString(AssetType type)
{
	switch (type)
	{
	case AssetType::Model:return "Model";
	case AssetType::Material:return "Material";
	case AssetType::World:return "World";
	case AssetType::Texture2D:return "Texture2D";
	case AssetType::TextureCube:return "TextureCube";
	case AssetType::Prefab:return "Prefab";
	case AssetType::Level:return "Level";
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
	HString name;
	HString suffix;
	HString relativePath;
	HString virtualPath;
	HString absPath;
	uint64_t byteSize;
	std::vector<AssetInfoBase*> refs;
	//用来暂时储存引用的guid和type,没有太多实际意义,通常是空的
	std::vector<AssetInfoRefTemp> refTemps;
	//
	bool bAssetLoad = false;
	virtual std::weak_ptr<class AssetObject> GetAssetData()const { return std::weak_ptr<class AssetObject>(); }
	inline const bool IsAssetLoad()const
	{
		return bAssetLoad;
	}
};

template<class T>
class AssetInfo : public AssetInfoBase
{
	std::shared_ptr<T> data = NULL;
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
	inline void ReleaseData(){
		data.reset();
		data = NULL;
		bAssetLoad = false;
	}
	inline void SetData(std::shared_ptr<T> newData){
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
	HBBR_API AssetInfoBase* ImportAssetInfo(AssetType type , HString sourceFile, HString contentPath);

	/* 删除资产 */
	HBBR_API void  DeleteAsset(HString filePath);

	/* 删除资产信息 */
	HBBR_API void RemoveAssetInfo(HGUID obj, AssetType type = AssetType::Unknow);

	HBBR_API inline const std::unordered_map<HGUID, AssetInfoBase*>& GetAssets(AssetType type)const { return _assets[(uint32_t)type]; }

	HBBR_API AssetInfoBase* GetAssetInfo(HGUID guid, AssetType type = AssetType::Unknow)const;

	/* 根据实际路径实际文件获取 */
	HBBR_API AssetInfoBase* GetAssetInfo(HString realAbsPath)const;

	/* 根据内容浏览器显示的文件名称(虚拟路径)查找 AssetInfo */
	HBBR_API AssetInfoBase* GetAssetInfo(AssetType type, HString contentBrowserFilePath)const;

	/* 根据内容浏览器显示的文件名称(虚拟路径)查找(非实际GUID的名称)GUID */
	HBBR_API HGUID GetAssetGUID(AssetType type,HString contentBrowserFilePath)const;

	template<class T>
	HBBR_INLINE std::weak_ptr<T> GetAsset(HGUID guid , AssetType type = AssetType::Unknow)
	{
		auto assetInfo = GetAssetInfo(guid , type);
		if (!assetInfo)
		{
			return std::weak_ptr<T>();
		}
		auto asset = reinterpret_cast<AssetInfo<T>*>(assetInfo);
		return asset->GetData();
	}

	//通过已知类型和guid加载资产
	template<class T>
	HBBR_INLINE std::weak_ptr<class AssetObject> LoadAsset(HGUID guid)
	{
		return T::LoadAsset(guid);
	}

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
