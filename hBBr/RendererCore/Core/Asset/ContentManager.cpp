#include "ContentManager.h"
#include "XMLStream.h"
#include "FileSystem.h"
#include "Asset/AssetObject.h"
#include "Asset/Model.h"
#include "Asset/Material.h"
#include "Asset/Texture2D.h"
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

//更新所有资产的引用
void ContentManager::UpdateAllAssetReference()
{
	for (int i = 0; i < (int)AssetType::MaxNum ; i++)
	{
		UpdateAssetReferenceByType((AssetType)i);
	}
}

//重载所有资产Info
void ContentManager::ReloadAllAssetInfos()
{
	//获取所有仓库目录
	//auto allRepositories = FileSystem::GetFilesBySuffix(FileSystem::GetContentAbsPath().c_str(), "repository");
	auto allRepositories = FileSystem::GetAllFolders(FileSystem::GetContentAbsPath().c_str());
	//读取仓库信息
	for (auto& i : allRepositories)
	{
		ReloadRepository(i.baseName);
	}
	UpdateAllAssetReference();
}

void ContentManager::Release()
{
	_assets.clear();
	_assets_repos.clear();
}

std::shared_ptr<AssetInfoBase> CreateInfo(AssetType type)
{
	std::shared_ptr<AssetInfoBase> result = nullptr;
	switch (type)//-------新增资产类型需要在这里添加实际对象,未来打包资产的时候会根据类型进行删留
	{
		case AssetType::Model:		result.reset(new AssetInfo<Model>());
		case AssetType::Material:		result.reset(new AssetInfo<Material>());
		case AssetType::Texture2D:	result.reset(new AssetInfo<Texture2D>());
		default:break;
	}
	return result;
}
//------------

void ContentManager::ReloadRepository(HString repositoryName)
{
	HString fullPath = FileSystem::Append(FileSystem::GetContentAbsPath(), repositoryName);
	fullPath = fullPath + ".repository";
	HString repositoryConfigPath = FileSystem::Append(fullPath, ".repository");
	pugi::xml_document doc;
	XMLStream::LoadXML(repositoryConfigPath.c_wstr(), doc);
	auto root = doc.child(TEXT("root"));
	for (pugi::xml_node i = root.first_child(); i; i = i.next_sibling())
	{
		HGUID guid;
		HString guidStr;
		XMLStream::LoadXMLAttributeString(i, L"GUID", guidStr);
		StringToGUID(guidStr.c_str(), &guid);
		AssetType type = (AssetType)i.attribute(L"Type").as_int();
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
		//设置参数
		info->type = type;
		info->guid = guid;
		info->displayName = i.attribute(L"Name").as_string();
		info->suffix = i.attribute(L"Format").as_string();
		info->absFilePath = FileSystem::Append(FileSystem::Append(fullPath , i.attribute(L"RPath").as_string()), guidStr) + "." +info->suffix;
		FileSystem::FixUpPath(info->absFilePath);
		info->absPath = FileSystem::Append(fullPath, i.attribute(L"RPath").as_string());
		FileSystem::FixUpPath(info->absPath);
		info->assetFilePath = FileSystem::GetRelativePath(info->absFilePath.c_str());
		FileSystem::FixUpPath(info->assetFilePath);
		info->assetPath = FileSystem::GetRelativePath(info->absPath.c_str());
		FileSystem::FixUpPath(info->assetPath);
		info->virtualPath = i.attribute(L"VPath").as_string();
		FileSystem::FixUpPath(info->virtualPath);
		info->virtualFilePath = i.attribute(L"VPath").as_string();
		info->virtualFilePath += info->displayName + "." + info->suffix;
		FileSystem::FixUpPath(info->virtualFilePath);
		info->repository = repositoryName;
		//读取引用关系
		info->refTemps.clear();
		for (auto j = i.first_child(); j; j = j.next_sibling())//<Ref>
		{
			HGUID subGuid;
			HString guidText = j.text().as_string();
			StringToGUID(guidText.c_str(), &guid);
			uint32_t typeIndex;
			XMLStream::LoadXMLAttributeUInt(j, L"Type", typeIndex);
			AssetInfoRefTemp newTemp;
			newTemp.guid = guid;
			newTemp.type = (AssetType)typeIndex;
			//先加载完所有资产信息,再处理引用关系!
			info->refTemps.push_back(newTemp);
		}

		if (checkExist.expired())
		{
			//1
			_assets[(uint32_t)type].emplace(info->guid, info);
			//2
			auto it = _assets_repos.find(repositoryName);
			if (it == _assets_repos.end())
			{
				std::unordered_map<HGUID, std::shared_ptr<AssetInfoBase>> newItem;
				newItem.emplace(info->guid, info);
				_assets_repos.emplace(repositoryName, newItem);
			}
			else
			{
				_assets_repos[repositoryName].emplace(info->guid, info);
			}
		}
		else
		{
			_assets[(uint32_t)type][info->guid] = info;
		}
	}
}

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

std::weak_ptr<AssetInfoBase> ContentManager::GetAssetInfo(HGUID guid, HString repositoryName) const
{
	auto it = _assets_repos.find(repositoryName);
	if (it != _assets_repos.end())
	{
		auto git = it->second.find(guid);
		if (git != it->second.end())
		{
			return git->second;
		}
	}
	return std::weak_ptr<AssetInfoBase>();
}

std::weak_ptr<AssetInfoBase> ContentManager::GetAssetInfo(HString virtualFilePath, AssetType type)const
{

	if (type != AssetType::Unknow && type < AssetType::MaxNum)
	{
		for (auto& i : _assets[(int)type])
		{
			if (i.second->virtualFilePath.IsSame(virtualFilePath, false))
			{
				return i.second;
			}
		}
	}
	else
	{
		for (auto& t : _assets)
		{
			for (auto& i : t)
			{
				if (i.second->virtualFilePath.IsSame(virtualFilePath, false))
				{
					return i.second;
				}
			}
		}
	}
	return std::weak_ptr<AssetInfoBase>();
}