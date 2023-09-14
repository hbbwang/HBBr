#include "ContentManager.h"
#include "XMLStream.h"
#include "FileSystem.h"
#include "Resource/ModelData.h"
#include "Component/Material.h"
std::unique_ptr<ContentManager> ContentManager::_ptr;

void ContentManager::UpdateReference(HGUID obj)
{

}

ContentManager::ContentManager()
{
	if (!XMLStream::LoadXML((FileSystem::GetContentAbsPath() + "ContentReference.xml").c_wstr(), _contentRefConfig))
	{
		MessageOut(HString("Can content reference config(Resource/Content/ContentReference.xml)!!.").c_str(), true, true, "255,0,0");
	}
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
}

void ContentManager::Release()
{
	ReleaseAssetsByType(AssetType::Model);
	ReleaseAssetsByType(AssetType::Material);
}

void ContentManager::ReleaseAssetsByType(AssetType type)
{
	switch (type)
	{
		case AssetType::Model:
		{
			for (auto i : _ModelAssets)
				if (i.second)
					delete i.second;
			_ModelAssets.clear();
			break;
		}
		case AssetType::Material:
		{
			for (auto i : _MaterialAssets)
				if (i.second)
					delete i.second;
			_MaterialAssets.clear();
			break;
		}
	}
}

void ContentManager::ReloadAssetInfos(AssetType type)
{
	HString typeName = GetAssetTypeString(type);

	pugi::xml_node subGroup = _contentRefConfig.child(TEXT("root")).child(typeName.c_wstr());
	if (subGroup)
	{
		switch (type)
		{
		case AssetType::Model:		ReleaseAssetsByType(AssetType::Model); break;
		case AssetType::Material:	ReleaseAssetsByType(AssetType::Material); break;
		}

		for (auto i = subGroup.first_child(); i; i = i.next_sibling())
		{
			AssetInfoBase* info = NULL;

			switch (type)
			{
			case AssetType::Model:		info = new AssetInfo<ModelData>(); break;
			case AssetType::Material:	info = new AssetInfo<Material>(); break;
			}

			info->type = type;
			HString guidStr;
			XMLStream::LoadXMLAttributeString(i, TEXT("GUID"), guidStr);
			StringToGUID(guidStr.c_str(), &info->guid);
			XMLStream::LoadXMLAttributeString(i, TEXT("Name"), info->name);
			XMLStream::LoadXMLAttributeString(i, TEXT("Path"), info->relativePath);

			switch (type)
			{
			case AssetType::Model:		_ModelAssets.emplace(info->guid, info); break;
			case AssetType::Material:	_MaterialAssets.emplace(info->guid, info); break;
			}

		}
	}	
}
