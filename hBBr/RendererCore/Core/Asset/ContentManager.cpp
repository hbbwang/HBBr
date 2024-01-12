#include "ContentManager.h"
#include "XMLStream.h"
#include "FileSystem.h"
#include "Asset/AssetObject.h"
#include "Asset/ModelData.h"
#include "Asset/Material.h"
#include "Asset/Texture.h"
#include "Asset/Level.h"
#include "Asset/World.h"
std::unique_ptr<ContentManager> ContentManager::_ptr;

ContentManager::ContentManager()
{
	_assets.resize((uint32_t)AssetType::MaxNum);
	//先加载对象,再确定引用关系
	ReloadAllAssetInfos();
}

ContentManager::~ContentManager()
{
	Release();
}

//更新某个类型的单个GUID的引用
void ContentManager::UpdateAssetReference(AssetType type, HGUID obj)
{
	UpdateAssetReference(_assets[(uint32_t)type][obj]);
}

//通过类型更新资产引用
void ContentManager::UpdateAssetReferenceByType(AssetType type)
{
	for (auto i : _assets[(uint32_t)type])
	{
		UpdateAssetReference(i.second);
	}
}

#pragma region Asset Type Function
//更新所有资产的引用
void ContentManager::UpdateAllAssetReference()
{
	UpdateAssetReferenceByType(AssetType::Model);
	UpdateAssetReferenceByType(AssetType::Material);
	UpdateAssetReferenceByType(AssetType::Texture2D);
}

//重载所有资产Info
void ContentManager::ReloadAllAssetInfos()
{
	auto files = FileSystem::GetFilesBySuffix(FileSystem::GetContentAbsPath().c_str(), "meta");
	for (auto& i : files)
	{
		ReloadAssetInfoByMetaFile(i.absPath);
	}
	UpdateAllAssetReference();
}

void ContentManager::Release()
{
	ReleaseAssetsByType(AssetType::Model, true);
	ReleaseAssetsByType(AssetType::Material, true);
	ReleaseAssetsByType(AssetType::Texture2D, true);
}

std::shared_ptr<AssetInfoBase> CreateInfo(AssetType type)
{
	std::shared_ptr<AssetInfoBase> result = nullptr;
	switch (type)//新增资产类型需要在这里添加实际对象,未来打包资产的时候会根据类型进行删留
	{
		case AssetType::Model:		result.reset(new AssetInfo<ModelData>());
		case AssetType::Material:	result.reset(new AssetInfo<Material>());
		case AssetType::Texture2D: result.reset(new AssetInfo<Texture>());
		default:break;
	}
	return result;
}

std::weak_ptr<AssetInfoBase> ContentManager::GetAssetInfo(HString assetPath) const
{
	if (!FileSystem::FileExist(assetPath))
	{
		assetPath = FileSystem::FillUpAssetPath(assetPath);
	}
	if (!FileSystem::FileExist(assetPath))
	{
		std::weak_ptr<AssetInfoBase>();
	}
	auto metaFilePath = assetPath;
	if (!metaFilePath.GetSuffix().IsSame("meta"))
	{
		metaFilePath += ".meta";
	}
	pugi::xml_document doc; 
	if (XMLStream::LoadXML(metaFilePath.c_wstr(), doc))
	{
		auto root = doc.child(L"root");
		HString guidStr;
		XMLStream::LoadXMLAttributeString(root, L"GUID", guidStr);
		HGUID guid(guidStr.c_str());
		int type;
		XMLStream::LoadXMLAttributeInt(root, L"Type", type);
		return  GetAssetInfo(guid, (AssetType(type)));
	}
	return std::weak_ptr<AssetInfoBase>();
}
#pragma endregion Asset Type Function(资产类型硬相关方法)

void ContentManager::UpdateAssetReference(HGUID obj)
{
	for (auto i : _assets)
	{
		auto it = i.find(obj);
		if (it != i.end())
		{
			UpdateAssetReference(it->second);
		}
	}
}

void ContentManager::ReleaseAssetsByType(AssetType type, bool bDestroy)
{
	if (bDestroy)
	{
		for (auto i : _assets[(uint32_t)type])
		{
			i.second.reset();
		}
		_assets[(uint32_t)type].clear();
	}
	else
	{
		for (auto i : _assets[(uint32_t)type])
		{
			i.second->ReleaseData();
		}
	}
}

void ContentManager::ReleaseAsset(AssetType type, HGUID obj)
{
	auto it = _assets[(uint32_t)type].find(obj);
	if (it != _assets[(uint32_t)type].end())
	{
		_assets[(uint32_t)type].erase(it);
	}
}

void ContentManager::UpdateAssetReference(std::weak_ptr<AssetInfoBase> info)
{
	if (!info.expired())
	{
		info.lock()->refs.clear();
		for (auto i : info.lock()->refTemps)
		{
			info.lock()->refs.push_back(_assets[(uint32_t)i.type][i.guid]);
		}
	}
}

std::weak_ptr<AssetInfoBase> ContentManager::ReloadAssetInfoByMetaFile(HString AbsPath)
{
	pugi::xml_document doc;
	if (!FileSystem::FileExist(AbsPath))
	{
		XMLStream::CreateXMLFile(AbsPath, doc);
	}
	else
	{
		XMLStream::LoadXML(AbsPath.c_wstr(), doc);
	}
	auto rootNode = doc.child(L"root");
	HGUID guid;
	HString guidStr;
	XMLStream::LoadXMLAttributeString(rootNode, L"GUID", guidStr);
	StringToGUID(guidStr.c_str(), &guid);
	AssetType type = (AssetType)rootNode.attribute(L"Type").as_int();

	//查看下是否已经加载过AssetInfo了,已经加载过就不需要创建了
	auto checkExist = GetAssetInfo(guid);
	bool bExist = !checkExist.expired();
	std::shared_ptr<AssetInfoBase> info = nullptr;
	if (checkExist.expired())
	{
		info = CreateInfo(type);
	}
	else
	{
		info = checkExist.lock();
	}

	//重新导入
	info->type = type;
	info->guid = guid;
	info->name = AbsPath.GetBaseName().GetBaseName();//xxx.fbx.meta
	info->absFilePath = AbsPath;
	FileSystem::FixUpPath(info->absFilePath);
	info->absPath = info->absFilePath.GetFilePath();
	FileSystem::FixUpPath(info->absPath);
	info->assetFilePath = FileSystem::GetRelativePath(AbsPath.c_str());
	FileSystem::FixUpPath(info->assetFilePath);
	info->assetPath = info->assetFilePath.GetFilePath();
	FileSystem::FixUpPath(info->assetPath);
	info->suffix = AbsPath.GetSuffix();
	info->metaFileAbsPath = AbsPath + ".meta";

	XMLStream::LoadXMLAttributeUint64(rootNode, L"ByteSize", info->byteSize);

	//Ref temps
	auto refNode = rootNode.child(L"Ref");
	for (auto j = refNode.first_child(); j; j = j.next_sibling())
	{
		HGUID guid;
		HString guidText = j.text().as_string();
		StringToGUID(guidText.c_str(), &guid);
		uint32_t typeIndex;
		XMLStream::LoadXMLAttributeUInt(j, L"Type", typeIndex);
		AssetInfoRefTemp newTemp;
		newTemp.guid = guid;
		newTemp.type = (AssetType)typeIndex;
		info->refTemps.push_back(newTemp);
	}

	if (checkExist.expired())
	{
		_assets[(uint32_t)type].emplace(info->guid, info);
	}
	return _assets[(uint32_t)type][info->guid];
}

std::weak_ptr<AssetInfoBase> ContentManager::CreateAssetInfo(HString AssetPath)
{
	auto contentPath = FileSystem::FillUpAssetPath(AssetPath);
	auto metaFile = contentPath + ".meta";

	{
		AssetSaveType save;
		save.metaAssetPath = metaFile;
		save.guid = CreateGUID();
		save.type = GetAssetTypeBySuffix(AssetPath.GetSuffix());
		save.byteSize = FileSystem::GetFileSize(AssetPath.c_str());
		SaveAssetInfo(save);
	}

	//重新导入并获取
	auto info = ReloadAssetInfoByMetaFile(metaFile);;

	return info;
}

void ContentManager::SaveAssetInfo(AssetInfoBase* info)
{
	if (info)
	{
		AssetSaveType save;
		save.metaAssetPath = info->metaFileAbsPath;
		save.guid = info->guid;
		save.type = info->type;
		save.byteSize = info->byteSize;
		save.refs = info->refs;
		SaveAssetInfo(save);
	}
}

void ContentManager::SaveAssetInfo(AssetSaveType save)
{
	pugi::xml_document doc;
	if (!FileSystem::FileExist(save.metaAssetPath))
	{
		XMLStream::CreateXMLFile(save.metaAssetPath, doc);
	}
	else
	{
		XMLStream::LoadXML(save.metaAssetPath.c_wstr(), doc);
	}

	auto rootNode = doc.child(L"root");
	//Type
	XMLStream::SetXMLAttribute(rootNode, L"Type", (int)save.type);
	//GUID
	XMLStream::SetXMLAttribute(rootNode, L"GUID", ((HString)save.guid.str().c_str()).c_wstr());
	//ByteSize
	XMLStream::SetXMLAttribute(rootNode, L"ByteSize", save.byteSize);
	
	//Reference
	auto refNode = XMLStream::CreateXMLNode(rootNode, L"Ref");
	for (auto& i : save.refs)
	{
		if (!i.expired())
		{
			//Ref Type
			XMLStream::SetXMLAttribute(rootNode, L"Type", (int)i.lock()->type);
			//Ref GUID
			XMLStream::SetXMLAttribute(rootNode, L"GUID", ((HString)i.lock()->guid.str().c_str()).c_wstr());
		}
	}

	doc.save_file(save.metaAssetPath.c_wstr());
}

void ContentManager::DeleteAsset(HString filePath)
{
	HString guidStr = filePath.GetBaseName();
	HGUID guid;
	StringToGUID(guidStr.c_str(), &guid);
	RemoveAssetInfo(guid);
	FileSystem::FileRemove(filePath.c_str());
	FileSystem::FileRemove((filePath+".meta").c_str());
}

void ContentManager::RemoveAssetInfo(HGUID obj, AssetType type )
{
	if (type == AssetType::Unknow)
	{
		for (auto i : _assets)
		{
			auto it = i.find(obj);
			if (it != i.end())
			{
				type = it->second->type;
				//移除引用关系
				for (auto i : it->second->refs)
				{
					if (!i.expired())
					{
						auto iit = std::remove_if(i.lock()->refs.begin(), i.lock()->refs.end(), [obj](std::weak_ptr<AssetInfoBase>& info) {
							return info.lock()->guid == obj;
							});
						if (iit != it->second->refs.end())
						{
							i.lock()->refs.erase(iit);
							SaveAssetInfo(i.lock().get());
						}
					}
				}
			}
		}
	}
	else
	{
		//移除资产信息
		auto it = _assets[(uint32_t)type].find(obj);
		if (it != _assets[(uint32_t)type].end())
		{
			//移除引用关系
			for (auto i : it->second->refs)
			{
				if (!i.expired())
				{
					auto iit = std::remove_if(i.lock()->refs.begin(), i.lock()->refs.end(), [obj](std::weak_ptr<AssetInfoBase>& info) {
						return info.lock()->guid == obj;
						});
					if (iit != it->second->refs.end())
					{
						i.lock()->refs.erase(iit);
						SaveAssetInfo(i.lock().get());
					}
				}
			}
		}
	}
}

std::weak_ptr<AssetInfoBase> ContentManager::GetAssetInfo(HGUID guid, AssetType type)const
{
	if (type != AssetType::Unknow)
	{
		auto it = _assets[(uint32_t)type].find(guid);
		if (it != _assets[(uint32_t)type].end())
		{
			return it->second;
		}
		return std::weak_ptr<AssetInfoBase>();
	}
	else
	{
		for (auto i : _assets)
		{
			auto it = i.find(guid);
			if (it != i.end())
			{
				return it->second;
			}
		}
		return std::weak_ptr<AssetInfoBase>();
	}
	return std::weak_ptr<AssetInfoBase>();
}

HGUID ContentManager::GetAssetGUID(HString assetPath)const
{
	auto info = GetAssetInfo(assetPath);
	if (!info.expired())
		return info.lock()->guid;
	else
		return HGUID();
}