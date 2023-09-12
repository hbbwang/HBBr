#include "ContentManager.h"
#include "XMLStream.h"
#include "FileSystem.h"

std::unique_ptr<ContentManager> ContentManager::_ptr;

void ContentManager::UpdateReference(HGUID obj)
{

}

ContentManager::ContentManager()
{
	pugi::xml_document _contentRefConfig;
	XMLStream::LoadXML((FileSystem::GetContentAbsPath()).c_wstr(), _contentRefConfig);
	pugi::xml_node root = _contentRefConfig.child(TEXT("root"));
	pugi::xml_node subGroup;
	//先加载对象,再确定引用关系
	//models
	subGroup = root.child(TEXT("Model"));
	for (auto i = subGroup.first_child(); subGroup != NULL; subGroup = subGroup.next_sibling())
	{
		AssetInfo info;
		info.type = AssetType::Model;
		HString guidStr;
		XMLStream::LoadXMLAttributeString(subGroup, TEXT("GUID"), guidStr);
		StringToGUID(guidStr.c_str(), &info.guid);
		XMLStream::LoadXMLAttributeString(subGroup, TEXT("Name"), info.name);
		XMLStream::LoadXMLAttributeString(subGroup, TEXT("Path"), info.relativePath);
		_ModelAssets.emplace(std::make_pair(info.guid, info));
	}
	//materials
}

void ContentManager::ReloadAllAssertInfos()
{

}
