﻿#include "ContentManager.h"
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
	_configPath = (FileSystem::GetContentAbsPath() + "ContentReference.xml");
	if (!XMLStream::LoadXML(_configPath.c_wstr(), _contentRefConfig))
	{
		MessageOut(HString("Can content reference config(Asset/Content/ContentReference.xml)!!.").c_str(), true, true, "255,0,0");
	}
	_assets.resize((uint32_t)AssetType::MaxNum);
	//先加载对象,再确定引用关系
	ReloadAllAssetInfos();
}

ContentManager::~ContentManager()
{
	Release();
}

//重载某个类型的所有Info
void ContentManager::ReloadAssetInfos(AssetType type)
{
	HString typeName = GetAssetTypeString(type);

	pugi::xml_node subGroup = _contentRefConfig.child(L"root").child(typeName.c_wstr());
	if (subGroup)
	{
		//卸载已经加载的信息
		ReleaseAssetsByType(type, false);

		//重新从引用配置文件里读取
		for (auto i = subGroup.first_child(); i; i = i.next_sibling())
		{
			ReloadAssetInfo(type, i);
		}
	}
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
	ReloadAssetInfos(AssetType::Model);
	ReloadAssetInfos(AssetType::Material);
	ReloadAssetInfos(AssetType::Texture2D);
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
	std::shared_ptr<AssetInfoBase> result = NULL;
	switch (type)//新增资产类型需要在这里添加实际对象,未来打包资产的时候会根据类型进行删留
	{
		case AssetType::Model:		result.reset(new AssetInfo<ModelData>());
		case AssetType::Material:	result.reset(new AssetInfo<Material>());
		case AssetType::Texture2D: result.reset(new AssetInfo<Texture>());
		default:break;
	}
	return result;
}

std::weak_ptr<AssetInfoBase> ContentManager::GetAssetInfo(AssetType type, HString contentBrowserFilePath) const
{
	HString filePath = FileSystem::GetFilePath(contentBrowserFilePath);
	HString fileBaseName = FileSystem::GetBaseName(contentBrowserFilePath);
	//获取路径下的同类型资产文件
	std::vector<FileEntry> files;
	if (type == AssetType::Model)
	{
		files = FileSystem::GetFilesBySuffix(filePath.c_str(), "fbx");
	}
	else if (type == AssetType::Material)
	{
		files = FileSystem::GetFilesBySuffix(filePath.c_str(), "mat");
	}
	else if (type == AssetType::Texture2D)
	{
		files = FileSystem::GetFilesBySuffix(filePath.c_str(), "dds");
	}

	if (type != AssetType::Unknow)
	{
		auto& map = _assets[(uint32_t)type];
		HString path = FileSystem::GetRelativePath(filePath.c_str());
		auto fit = std::find_if(map.begin(), map.end(), [&](const std::pair<HGUID, std::shared_ptr<AssetInfoBase>>& item) {
			return item.second->relativePath == path && fileBaseName == item.second->name;
			});
		if (fit != map.end())
		{
			return fit->second;
		}
	}
	MessageOut((HString(L"Error, the asset cannot be found. Please check whether the Asset directory is complete or the asset is missing.\n错误,无法找到资产,请检查Asset目录是否完整或资产缺失:\n") + contentBrowserFilePath).c_str(), false, false, "255,0,0");
	return std::weak_ptr<AssetInfoBase>();
}

std::weak_ptr<AssetInfoBase> ContentManager::GetAssetInfo(HString realAbsPath) const
{
	HString guidStr = FileSystem::GetBaseName(realAbsPath);
	HString suffix = FileSystem::GetFileExt(realAbsPath);
	HGUID guid;
	StringToGUID(guidStr.c_str(), &guid);
	AssetType type = AssetType::Unknow;
	if (suffix.IsSame("fbx"))
	{
		type = AssetType::Model;
	}
	else if (suffix.IsSame("dds"))
	{
		type = AssetType::Texture2D;
	}
	else if (suffix.IsSame("mat"))
	{
		type = AssetType::Material;
	}
	return GetAssetInfo(guid, type);
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

void ContentManager::ReloadAssetInfo(AssetType type , pugi::xml_node & i)
{
	HGUID guid;
	HString guidStr;
	XMLStream::LoadXMLAttributeString(i, L"GUID", guidStr);
	StringToGUID(guidStr.c_str(), &guid);

	//查看下是否已经加载过AssetInfo了,已经加载过就不需要创建了
	auto checkExist = GetAssetInfo(guid, type);
	bool bExist = !checkExist.expired();
	std::shared_ptr<AssetInfoBase> info = NULL;
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
	XMLStream::LoadXMLAttributeString(i, L"Name", info->name);
	XMLStream::LoadXMLAttributeString(i, L"Path", info->relativePath);
	FileSystem::CorrectionPath(info->relativePath);
	XMLStream::LoadXMLAttributeString(i, L"Suffix", info->suffix);
	XMLStream::LoadXMLAttributeUint64(i, L"ByteSize", info->byteSize);

	HString path = FileSystem::GetProgramPath() + info->relativePath;
	info->absPath = path + "./" + guidStr + info->suffix;
	info->virtualPath = info->relativePath + "./" + info->name;

	FileSystem::CorrectionPath(info->absPath);
	FileSystem::CorrectionPath(info->virtualPath);

	//Ref temps
	for (auto j = i.first_child(); j; j = j.next_sibling())
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
}

std::weak_ptr<AssetInfoBase> ContentManager::ImportAssetInfo(AssetType type, HString name, HString suffix, HString assetPath)
{
	return ImportAssetInfo(type, "./" + name + "." + suffix, assetPath);
}

std::weak_ptr<AssetInfoBase> ContentManager::ImportAssetInfo(AssetType type, HString sourcePath,HString contentPath)
{
	FileSystem::CorrectionPath(sourcePath);
	FileSystem::NormalizePath(contentPath);
	HString typeName = GetAssetTypeString(type);
	HString name = sourcePath.GetBaseName();
	HString suffix = sourcePath.GetSuffix();
	HString relativePath = FileSystem::GetRelativePath(contentPath.c_str());
	relativePath.Replace("\\", "/");
	auto root = _contentRefConfig.child(L"root");
	pugi::xml_node TypeNode = root.child(typeName.c_wstr());
	pugi::xml_node item;
	bool bExist = false;
	if (!TypeNode)
	{
		TypeNode = root.append_child(typeName.c_wstr());
		item = TypeNode.append_child(L"Item");
	}
	else
	{
		//find exist
		for (auto n = TypeNode.first_child(); n != NULL; n = n.next_sibling())
		{
			HString n_name = n.attribute(L"Name").as_string();
			HString n_path = n.attribute(L"Path").as_string();
			if (name.IsSame(n_name) && relativePath.IsSame(n_path))
			{
				item = n;
				bExist = true;
				break;
			}
		}
		if (!bExist)
		{
			item = TypeNode.append_child(L"Item");
		}
	}
	//配置xml
	HGUID guid;
	if (!bExist)
	{
		guid = CreateGUID();
		HString guidStr = GUIDToString(guid);
		item.append_attribute(L"GUID").set_value(guidStr.c_wstr());
		item.append_attribute(L"Name").set_value(name.c_wstr());
		item.append_attribute(L"Suffix").set_value(suffix.c_wstr());
		item.append_attribute(L"Path").set_value(relativePath.c_wstr());
		if (FileSystem::FileExist(sourcePath.c_str()))
		{
			item.append_attribute(L"ByteSize").set_value(FileSystem::GetFileSize(sourcePath.c_str()));
		}
	}
	else
	{
		HString guidStr = item.attribute(L"GUID").as_string();
		StringToGUID(guidStr.c_str(), &guid);
		item.attribute(L"Path").set_value(relativePath.c_wstr());
		if (FileSystem::FileExist(sourcePath.c_str()))
		{
			item.attribute(L"ByteSize").set_value(FileSystem::GetFileSize(sourcePath.c_str()));
		}
	}

	//重新导入
	ReloadAssetInfo(type,item);

	//保存
	_contentRefConfig.save_file(_configPath.c_wstr());
	return GetAssetInfo(guid, type);
}

void ContentManager::DeleteAsset(HString filePath)
{
	HString guidStr = filePath.GetBaseName();
	HGUID guid;
	StringToGUID(guidStr.c_str(), &guid);
	RemoveAssetInfo(guid);
	FileSystem::FileRemove(filePath.c_str());
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
						}
					}
				}
				HString typeName = GetAssetTypeString(type);
				HString guidStr = GUIDToString(obj);
				pugi::xml_node subGroup = _contentRefConfig.child(L"root").child(typeName.c_wstr());
				if (subGroup)
				{
					auto item = subGroup.find_child_by_attribute(L"GUID", guidStr.c_wstr());
					subGroup.remove_child(item);
				}
			}
		}
	}
	else
	{
		HString typeName = GetAssetTypeString(type);
		HString guidStr = GUIDToString(obj);
		pugi::xml_node subGroup = _contentRefConfig.child(L"root").child(typeName.c_wstr());
		if (subGroup)
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
						}
					}				
				}
				auto item = subGroup.find_child_by_attribute(L"GUID", guidStr.c_wstr());
				subGroup.remove_child(item);
			}
		}
	}
	//保存
	_contentRefConfig.save_file(_configPath.c_wstr());
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

HGUID ContentManager::GetAssetGUID(AssetType type, HString contentBrowserFilePath)const
{
	auto info = GetAssetInfo(type, contentBrowserFilePath);
	if (!info.expired())
		return info.lock()->guid;
	else
		return HGUID();
}