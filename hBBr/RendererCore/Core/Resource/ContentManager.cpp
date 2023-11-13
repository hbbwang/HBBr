#include "ContentManager.h"
#include "XMLStream.h"
#include "FileSystem.h"
#include "Resource/ResourceObject.h"
#include "Resource/ModelData.h"
#include "Resource/Material.h"
#include "Resource/Texture.h"
std::unique_ptr<ContentManager> ContentManager::_ptr;

ContentManager::ContentManager()
{
	_configPath = (FileSystem::GetContentAbsPath() + "ContentReference.xml");
	if (!XMLStream::LoadXML(_configPath.c_wstr(), _contentRefConfig))
	{
		MessageOut(HString("Can content reference config(Resource/Content/ContentReference.xml)!!.").c_str(), true, true, "255,0,0");
	}
	_assets.resize((uint32_t)AssetType::MaxNum);
	//先加载对象,再确定引用关系
	ReloadAllAssetInfos();
}

ContentManager::~ContentManager()
{
	Release();
}

void ContentManager::ReloadAllAssetInfos()
{
	ReloadAssetInfos(AssetType::Model);
	ReloadAssetInfos(AssetType::Material);
	ReloadAssetInfos(AssetType::Texture2D);
	UpdateAllAssetReference();
}

void ContentManager::UpdateAllAssetReference()
{
	UpdateAssetReferenceByType(AssetType::Model);
	UpdateAssetReferenceByType(AssetType::Material);
	UpdateAssetReferenceByType(AssetType::Texture2D);
}

void ContentManager::UpdateAssetReferenceByType(AssetType type)
{
	for (auto i : _assets[(uint32_t)type])
	{
		UpdateAssetReference(i.second);
	}
}

void ContentManager::UpdateAssetReference(AssetType type , HGUID obj)
{
	UpdateAssetReference(_assets[(uint32_t)type][obj]);
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

void ContentManager::Release()
{
	ReleaseAssetsByType(AssetType::Model);
	ReleaseAssetsByType(AssetType::Material);
	ReleaseAssetsByType(AssetType::Texture2D);
}

void ContentManager::ReleaseAssetsByType(AssetType type)
{
	for (auto i : _assets[(uint32_t)type])
		if (i.second)
			delete i.second;
	_assets[(uint32_t)type].clear();
}

void ContentManager::ReleaseAsset(AssetType type, HGUID obj)
{
	auto it = _assets[(uint32_t)type].find(obj);
	if (it != _assets[(uint32_t)type].end())
	{
		_assets[(uint32_t)type].erase(it);
	}
}

AssetInfoBase* CreateInfo(AssetType type)
{
	switch (type)//新增资产类型需要在这里添加实际对象,未来打包资产的时候会根据类型进行删留
	{
		case AssetType::Model:		return new AssetInfo<ModelData>(); 
		case AssetType::Material:	return new AssetInfo<Material>();
		case AssetType::Texture2D:	return new AssetInfo<Texture>();
		default:break;
	}
	return NULL;
}

void ContentManager::ReloadAssetInfos(AssetType type)
{
	HString typeName = GetAssetTypeString(type);

	pugi::xml_node subGroup = _contentRefConfig.child(L"root").child(typeName.c_wstr());
	if (subGroup)
	{
		//卸载已经加载的信息
		ReleaseAssetsByType(type);

		//重新从引用配置文件里读取
		for (auto i = subGroup.first_child(); i; i = i.next_sibling())
		{
			ReloadAssetInfo(type, i);
		}
	}	
}

void ContentManager::UpdateAssetReference(AssetInfoBase* info)
{
	info->refs.clear();
	for (auto i : info->refTemps)
	{
		info->refs.push_back(_assets[(uint32_t)i.type][i.guid]);
	}
}

void ContentManager::ReloadAssetInfo(AssetType type , pugi::xml_node & i)
{
	//重新导入
	AssetInfoBase* info = CreateInfo(type);
	info->type = type;
	HString guidStr;
	XMLStream::LoadXMLAttributeString(i, L"GUID", guidStr);
	StringToGUID(guidStr.c_str(), &info->guid);
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
	//尝试卸载旧资产
	ReleaseAsset(type, info->guid);
	//
	_assets[(uint32_t)type].emplace(info->guid, info);
}

AssetInfoBase* ContentManager::ImportAssetInfo(AssetType type, HString sourcePath,HString contentPath)
{
	FileSystem::CorrectionPath(sourcePath);
	contentPath.Replace("\\", "/");
	HString typeName = GetAssetTypeString(type);
	HString name = sourcePath.GetBaseName();
	HString suffix = sourcePath.GetSuffix();
	HString path = FileSystem::GetRelativePath(contentPath.c_str());
	path.Replace("\\", "/");
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
			if (name.IsSame(n_name) && path.IsSame(n_path))
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
		item.append_attribute(L"Path").set_value(path.c_wstr());
		item.append_attribute(L"ByteSize").set_value(FileSystem::GetFileSize(sourcePath.c_str()));
	}
	else
	{
		HString guidStr = item.attribute(L"GUID").as_string();
		StringToGUID(guidStr.c_str(), &guid);
		item.attribute(L"Path").set_value(path.c_wstr());
		item.attribute(L"ByteSize").set_value(FileSystem::GetFileSize(sourcePath.c_str()));
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
					auto iit = std::remove_if(i->refs.begin(), i->refs.end(), [obj](AssetInfoBase*& info) {
						return info->guid == obj;
						});
					if (iit != it->second->refs.end())
					{
						i->refs.erase(iit);
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
					auto iit = std::remove_if(i->refs.begin(), i->refs.end(), [obj](AssetInfoBase*& info) {
						return info->guid == obj;
						});
					if (iit != it->second->refs.end())
					{
						i->refs.erase(iit);
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

AssetInfoBase* ContentManager::GetAssetInfo(HGUID guid, AssetType type)const 
{
	if (type != AssetType::Unknow)
	{
		auto it = _assets[(uint32_t)type].find(guid);
		if (it != _assets[(uint32_t)type].end())
		{
			return it->second;
		}
		return NULL;
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
		return NULL;
	}
	return NULL;
}

AssetInfoBase* ContentManager::GetAssetInfo(AssetType type, HString contentBrowserFilePath) const
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
		auto fit = std::find_if(map.begin(), map.end(), [&](const std::pair<HGUID, AssetInfoBase*>& item) {
			return item.second->relativePath == path && fileBaseName == item.second->name;
			});
		if (fit != map.end())
		{
			return fit->second;
		}
	}
	MessageOut((HString(L"Error, the asset cannot be found. Please check whether the Resource directory is complete or the asset is missing.\n错误,无法找到资产,请检查Resource目录是否完整或资产缺失:\n") + contentBrowserFilePath).c_str(), false, false, "255,0,0");
	return NULL;
}

AssetInfoBase* ContentManager::GetAssetInfo(HString realAbsPath) const
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
	return GetAssetInfo(guid , type);
}

HGUID ContentManager::GetAssetGUID(AssetType type, HString contentBrowserFilePath)const
{
	auto info = GetAssetInfo(type, contentBrowserFilePath);
	if (info != NULL)
		return info->guid;
	else
		return HGUID();
}