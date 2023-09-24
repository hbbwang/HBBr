#include "ContentManager.h"
#include "XMLStream.h"
#include "FileSystem.h"
#include "Resource/ModelData.h"
#include "Component/Material.h"
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

	UpdateAllAssetReference();
}

void ContentManager::UpdateAllAssetReference()
{
	UpdateAssetReferenceByType(AssetType::Model);
	UpdateAssetReferenceByType(AssetType::Material);
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
	}
	return NULL;
}

void ContentManager::ReloadAssetInfos(AssetType type)
{
	HString typeName = GetAssetTypeString(type);

	pugi::xml_node subGroup = _contentRefConfig.child(TEXT("root")).child(typeName.c_wstr());
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
	XMLStream::LoadXMLAttributeString(i, TEXT("GUID"), guidStr);
	StringToGUID(guidStr.c_str(), &info->guid);
	XMLStream::LoadXMLAttributeString(i, TEXT("Name"), info->name);
	XMLStream::LoadXMLAttributeString(i, TEXT("Path"), info->relativePath);
	XMLStream::LoadXMLAttributeString(i, TEXT("Suffix"), info->suffix);
	XMLStream::LoadXMLAttributeUint64(i, TEXT("ByteSize"), info->byteSize);
	//Ref temps
	for (auto j = i.first_child(); j; j = j.next_sibling())
	{
		HGUID guid;
		HString guidText = j.text().as_string();
		StringToGUID(guidText.c_str(), &guid);
		uint32_t typeIndex;
		XMLStream::LoadXMLAttributeUInt(j, TEXT("Type"), typeIndex);
		AssetInfoRefTemp newTemp;
		newTemp.guid = guid;
		newTemp.type = (AssetType)typeIndex;
		info->refTemps.push_back(newTemp);
	}
	//卸载
	ReleaseAsset(type, info->guid);
	//
	_assets[(uint32_t)type].emplace(info->guid, info);
}

AssetInfoBase* ContentManager::ImportAssetInfo(AssetType type, HString sourcePath,HString contentPath)
{
	AssetInfoBase* result = NULL;
	HString typeName = GetAssetTypeString(type);
	auto root = _contentRefConfig.child(TEXT("root"));
	pugi::xml_node subGroup = root.child(typeName.c_wstr());
	if (!subGroup)
	{
		subGroup = root.append_child(typeName.c_wstr());
	}
	//配置xml
	auto item = subGroup.append_child(TEXT("Item"));
	auto guid = CreateGUID();
	HString guidStr = GUIDToString(guid);
	HString name = sourcePath.GetBaseName();
	HString suffix = sourcePath.GetSuffix();
	HString path = FileSystem::GetRelativePath(contentPath.GetFilePath().c_str());
	item.append_attribute(TEXT("GUID")).set_value(guidStr.c_wstr());
	item.append_attribute(TEXT("Name")).set_value(name.c_wstr());
	item.append_attribute(TEXT("Suffix")).set_value(suffix.c_wstr());
	item.append_attribute(TEXT("Path")).set_value(path.c_wstr());
	item.append_attribute(TEXT("ByteSize")).set_value(FileSystem::GetFileSize(sourcePath.c_str()));

	//重新导入
	ReloadAssetInfo(type,item);

	//
	sourcePath.CorrectionPath();
	contentPath += "/";
	contentPath += guidStr + "." + suffix;
	contentPath.CorrectionPath();
	FileSystem::FileCopy(sourcePath.c_str(), contentPath .c_str());

	//保存
	_contentRefConfig.save_file(_configPath.c_wstr());
	return result;
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
				pugi::xml_node subGroup = _contentRefConfig.child(TEXT("root")).child(typeName.c_wstr());
				if (subGroup)
				{
					auto item = subGroup.find_child_by_attribute(TEXT("GUID"), guidStr.c_wstr());
					subGroup.remove_child(item);
				}
			}
		}
	}
	else
	{
		HString typeName = GetAssetTypeString(type);
		HString guidStr = GUIDToString(obj);
		pugi::xml_node subGroup = _contentRefConfig.child(TEXT("root")).child(typeName.c_wstr());
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
				auto item = subGroup.find_child_by_attribute(TEXT("GUID"), guidStr.c_wstr());
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