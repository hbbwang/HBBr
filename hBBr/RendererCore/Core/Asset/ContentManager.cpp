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
std::vector<std::weak_ptr<AssetInfoBase>> ContentManager::_dirtyAssets;
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
	_assets_vf.clear();
}

#if IS_EDITOR

bool ContentManager::AssetImport(HString repositoryName , std::vector<AssetImportInfo> importFiles, std::vector<std::weak_ptr<AssetInfoBase>>* out)
{
	HString contentPath = FileSystem::GetContentAbsPath();
	HString repositoryPath = FileSystem::Append(contentPath, repositoryName);
	HString repositoryXMLPath = FileSystem::Append(repositoryPath,".repository");
	pugi::xml_document doc;
	struct resultFind
	{
		HGUID guid;
		HString VirtualPath;
	};
	std::vector<resultFind> guids;
	if (out != nullptr)
	{
		guids.reserve(importFiles.size());
		out->reserve(importFiles.size());
	}
	if (XMLStream::LoadXML(repositoryXMLPath.c_wstr(), doc))
	{
		auto root = doc.child(TEXT("root"));
		for (auto& i : importFiles)
		{
			HString suffix = FileSystem::GetFileExt(i.absAssetFilePath);
			HString baseName = FileSystem::GetBaseName(i.absAssetFilePath);
			HString fileName = FileSystem::GetFileName(i.absAssetFilePath);

			//查看下当前虚拟目录下有没有相同名字和类型的文件，有就进行覆盖，没有才新增
			bool bFound = false; 
			HGUID guid;
			HString guidStr;
			{
				HString newVirtualFullPath = FileSystem::Append(i.virtualPath, fileName);
				HString cVP = i.virtualPath;
				FileSystem::ClearPathSeparation(cVP);
				FileSystem::ClearPathSeparation(newVirtualFullPath); 
				auto fit = _assets_vf.find(cVP);
				if (fit != _assets_vf.end())
				{
					for (auto& fit_i : fit->second.assets)
					{
						HString checkFilePath = fit_i.second->virtualFilePath;
						FileSystem::ClearPathSeparation(checkFilePath);
						if (checkFilePath.IsSame(newVirtualFullPath))
						{
							bFound = true;
							guid = fit_i.second->guid;
							guidStr = fit_i.second->guid.str().c_str();
						}
					}
				}
			}
			if (!bFound)
			{
				guidStr = CreateGUIDAndString(guid);
			}
			//

			AssetType type = AssetType::Unknow;
			HString assetTypeName = "Unknow";
			//----------------------------------------Model
			if (suffix.IsSame("fbx", false))
			{
				type = AssetType::Model;
				HString savePath = FileSystem::Append(repositoryPath, "Model");
				savePath = FileSystem::Append(savePath, guidStr + "." + suffix);
				//复制fbx到项目目录
				FileSystem::FileCopy(i.absAssetFilePath.c_str(), savePath.c_str());
			}
			//----------------------------------------Texture
			else if (suffix.IsSame("png", false)
				|| suffix.IsSame("tga", false)
				|| suffix.IsSame("jpg", false)
				|| suffix.IsSame("bmp", false)
				)
			{
				type = AssetType::Texture2D;
				HString savePath = FileSystem::Append(repositoryPath, "Texture");

			}
			//----------------------------------------Material
			else if (suffix.IsSame("mat", false))
			{
				//材质球只能在编辑器创建,无法导入
				type = AssetType::Material;
			}
			//导入AssetInfo
			if(type != AssetType::Unknow)
			{
				assetTypeName = GetAssetTypeString(type);
				//auto newItem = root.append_child(TEXT("Item"));	
				auto newItem = pugi::xml_node();
				if (bFound)
				{
					bool bFindGUID = false;
					for (auto n = root.first_child(); n; n = n.next_sibling())
					{
						if (guidStr.IsSame(n.attribute(TEXT("GUID")).as_string()))
						{
							bFindGUID = true;
							newItem = XMLStream::CreateXMLNode(root, TEXT("Item"), true);
						}
					}
					if (!bFindGUID)
					{
						//仓库文件中没找到该GUID的Item，重新添加一份上去。
						newItem = XMLStream::CreateXMLNode(root, TEXT("Item"), false);
					}
				}
				else
				{
					newItem = XMLStream::CreateXMLNode(root, TEXT("Item"), false);
				}
				XMLStream::SetXMLAttribute(newItem,TEXT("GUID"), guidStr.c_wstr());
				XMLStream::SetXMLAttribute(newItem, TEXT("Type"), (int)type);
				XMLStream::SetXMLAttribute(newItem, TEXT("Name"), baseName.c_wstr());
				XMLStream::SetXMLAttribute(newItem, TEXT("Format"), suffix.c_wstr());
				XMLStream::SetXMLAttribute(newItem, TEXT("VPath"), i.virtualPath.c_wstr());
				XMLStream::SetXMLAttribute(newItem, TEXT("RPath"), (assetTypeName + "/").c_wstr());
				if (!doc.save_file(repositoryXMLPath.c_wstr()))
				{
					MessageOut("Save repository xml failed.", false, true, "255,255,0");
				}
				resultFind newFind;
				newFind.guid = guid;
				newFind.VirtualPath = i.virtualPath;
				guids.push_back(newFind);
			}
			else
			{
				MessageOut("Asset Import Failed.Unknow asset type.",false, false,"255,255,0");
				return false;
			}
		}
		ReloadRepository(repositoryName);
		UpdateAllAssetReference();
		if (out != nullptr)
		{
			for (auto& i : guids)
			{
				auto assets = GetAssetsByVirtualFolder(i.VirtualPath);
				auto resultAsset = assets.find(i.guid);
				if (resultAsset != assets.end())
				{
					out->push_back(resultAsset->second);
				}
			}
		}
	}
	return true;
}

void ContentManager::AssetDelete(std::vector<AssetInfoBase*> assetInfos, bool isRemoveTheEmptyVirtualFolder, bool messageBox)
{
	bool bSure = true;
	if (messageBox)
	{
		MessageOut("You are deleting assets, are you sure?");
		HString msg;
		for (auto& i : assetInfos)
		{
			msg += i->virtualFilePath + "\n";
		}
		msg = HString("You are deleting assets, are you sure?\nPlease check the asset reference relationship before deleting.\n\n") + msg;
		SDL_MessageBoxButtonData buttons[] = {
			{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Cancel" },
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Contunue" },
		};
		SDL_MessageBoxData messageboxdata = {
			SDL_MESSAGEBOX_INFORMATION,    // 消息框类型
			NULL,                           // 父窗口
			"AssetDelete",               // 消息框标题
			msg.c_str(),					// 消息框内容
			SDL_arraysize(buttons), // 按钮数量
			buttons,                        // 按钮数据
			NULL                            // 颜色方案（使用默认颜色）
		};
		int buttonid;
		if (SDL_ShowMessageBox(&messageboxdata, &buttonid) >= 0) {
			if (buttonid == 0) {
				bSure = true;
			}
			else {
				bSure = false;
			}
		}
	}
	if (bSure)
	{
		std::vector<std::shared_ptr<AssetInfoBase>> assetInfosPtr;
		std::vector<HString>repositories;
		repositories.reserve(assetInfos.size());
		for (auto& i : assetInfos)
		{
			//资产删除不会帮忙处理资产引用关系(包括场景的)，如果删错了
			//编辑器会直接告知用户引用错误信息，让用户自行处理(不会崩溃)
			//所以删除前最好使用 [引用关系查看器] 检查好 
			HString repository = i->repository;
			auto ssit = std::find_if(repositories.begin(), repositories.end(), [repository](HString & s) {
				return s == repository;
			});
			if (ssit == repositories.end())
			{
				repositories.push_back(repository);
			}
			//删除实际资产
			FileSystem::FileRemove(i->absFilePath.c_str());
			//移除.repository内储存的Item信息
			HString contentPath = FileSystem::GetContentAbsPath();
			HString repositoryPath = FileSystem::Append(contentPath, repository);
			HString repositoryXMLPath = FileSystem::Append(repositoryPath, ".repository");
			pugi::xml_document doc;
			if (XMLStream::LoadXML(repositoryXMLPath.c_wstr(), doc))
			{
				auto root = doc.child(TEXT("root"));
				HGUID guid;
				for (auto n = root.first_child(); n; n = n.next_sibling())
				{
					HString guidStr = n.attribute(TEXT("GUID")).as_string();
					StringToGUID(guidStr.c_str(), &guid);
					if (guid == i->guid)
					{
						root.remove_child(n);
						break;
					}
				}
				if (!doc.save_file(repositoryXMLPath.c_wstr()))
				{
					MessageOut("Save repository xml failed.", false, true, "255,255,0");
					continue;
				}


				//收集临时共享指针,防止因为earse导致指针释放
				auto assetIt = _assets[(int)i->type].find(guid);
				if (assetIt != _assets[(int)i->type].end())
				{
					assetInfosPtr.push_back(assetIt->second);
				}

			}
		}

		//移除内存中的AssetInfo
		{
			for (auto& i : assetInfosPtr)
			{
				_assets[(int)i->type].erase(i->guid);
				_assets_repos[i->repository].erase(i->guid);
				HString vpvf = i->virtualPath;
				FileSystem::ClearPathSeparation(vpvf);
				_assets_vf[vpvf].assets.erase(i->guid);
				if (isRemoveTheEmptyVirtualFolder)
				{
					//如果更改的过程发现虚拟目录空了,就直接删了吧
					if (_assets_vf[vpvf].assets.size() <= 0)
					{
						_assets_vf.erase(vpvf);
					}
				}
			}		
		}

		//重新刷新仓库和引用
		for (auto& r : repositories)
		{
			ReloadRepository(r);
		}
		UpdateAllAssetReference();
	}
}

void ContentManager::SetNewVirtualPath(std::vector<AssetInfoBase*> assetInfos, HString newVirtualPath, bool bDeleteEmptyFolder)
{
	std::vector<HString> repositories;
	for (auto& i : assetInfos)
	{
		HString assetRepository = i->repository;
		HString oldVirtualPath = i->virtualPath;
		//修改info的虚拟路径
		i->virtualPath = newVirtualPath;
		i->virtualFilePath = FileSystem::Append(newVirtualPath, i->displayName + "." + i->suffix);
		i->virtualFilePath.Replace("\\", "/");
		auto it = std::find_if(repositories.begin(), repositories.end(), [assetRepository](HString & r) {
			return r.IsSame(assetRepository);
			});
		if (it == repositories.end())
		{
			repositories.push_back(assetRepository);
		}
		//保存进.repository
		SaveAssetInfo(i);
		{
			//添加
			HString vpf = i->virtualPath;
			HString vpf_old = oldVirtualPath;
			FileSystem::ClearPathSeparation(vpf);
			FileSystem::ClearPathSeparation(vpf_old);
			auto vfit = _assets_vf.find(vpf);
			if (vfit == _assets_vf.end())
			{
				VirtualFolder newVF;
				newVF.assets.emplace(i->guid, _assets_vf[vpf_old].assets[i->guid]);
				newVF.FolderName = FileSystem::GetFileName(i->virtualPath);
				newVF.Path = i->virtualPath;
				_assets_vf.emplace(vpf, newVF);
			}
			else
			{
				vfit->second.assets.emplace(i->guid, _assets_vf[vpf_old].assets[i->guid]);
			}
			//移除内存，该函数修改了虚拟路径，所以只需要处理虚拟路径的_assets_vf就行了
			_assets_vf[vpf_old].assets.erase(i->guid);

			if (bDeleteEmptyFolder)
			{
				//如果更改的过程发现老的虚拟目录空了,就直接删了吧
				if (_assets_vf[vpf_old].assets.size() <= 0)
				{
					_assets_vf.erase(vpf_old);
				}
			}
		}
	}

	//for (auto& i : repositories)
	//{
	//	//重新加载
	//	ReloadRepository(i);
	//}

	//刷新引用(只是改个虚拟路径不需要刷新引用)
	//UpdateAllAssetReference();
}

void ContentManager::SaveAssetInfo(AssetInfoBase* assetInfo)
{
	if (assetInfo)
	{
		HString repositoryXMLPath = FileSystem::GetRepositoryXMLAbsPath(assetInfo->repository);
		pugi::xml_document doc;
		pugi::xml_node target = pugi::xml_node();
		if (XMLStream::LoadXML(repositoryXMLPath.c_wstr(), doc))
		{
			auto root = doc.child(TEXT("root"));
			HGUID guid;
			HString guidStr;
			for (auto n = root.first_child(); n; n = n.next_sibling())
			{	
				guidStr = n.attribute(TEXT("GUID")).as_string();
				StringToGUID(guidStr.c_str(), &guid);
				if (guid == assetInfo->guid)
				{
					target = n;
					break;
				}
			}
			if (!target)
			{
				target = XMLStream::CreateXMLNode(root, TEXT("item"));
			}
			//设置属性,保存XML
			HString assetTypeName = GetAssetTypeString(assetInfo->type);
			XMLStream::SetXMLAttribute(target, TEXT("GUID"), guidStr.c_wstr());
			XMLStream::SetXMLAttribute(target, TEXT("Type"), (int)assetInfo->type);
			XMLStream::SetXMLAttribute(target, TEXT("Name"), assetInfo->displayName.c_wstr());
			XMLStream::SetXMLAttribute(target, TEXT("VPath"), assetInfo->virtualPath.c_wstr());
			XMLStream::SetXMLAttribute(target, TEXT("Format"), assetInfo->suffix.c_wstr());
			XMLStream::SetXMLAttribute(target, TEXT("RPath"), (assetTypeName + "/").c_wstr());
			if (!doc.save_file(repositoryXMLPath.c_wstr()))
			{
				MessageOut("Save repository xml failed.", false, true, "255,255,0");
			}

			//保存了的资产自动退出Dirty Mark
			{
				auto vit = std::remove_if(_dirtyAssets.begin(), _dirtyAssets.end(), [assetInfo](std::weak_ptr<AssetInfoBase>& a) {
					return a.lock()->guid == assetInfo->guid;
					});
				if (vit != _dirtyAssets.end())
				{
					_dirtyAssets.erase(vit);
				}
			}

		}
	}
}

void ContentManager::SetVirtualName(AssetInfoBase* assetInfo, HString newName, bool bSave)
{
	if (assetInfo)
	{
		//更新asset info
		assetInfo->displayName = newName;
		assetInfo->virtualFilePath = FileSystem::Append(assetInfo->virtualPath, assetInfo->displayName + "." + assetInfo->suffix);
		assetInfo->virtualFilePath.Replace("\\", "/");
		UpdateToolTips(assetInfo);
		if (bSave)
		{
			SaveAssetInfo(assetInfo);
		}
	}
}

void ContentManager::MarkAssetDirty(std::weak_ptr<AssetInfoBase> asset)
{
	asset.lock()->bDirty = true;
	_dirtyAssets.push_back(asset);
}

void  ContentManager::ClearDirtyAssets()
{
	for (auto& i : _dirtyAssets)
	{
		if (!i.expired())
		{
			i.lock()->bDirty = false;
		}
	}
	_dirtyAssets.clear();
}

void ContentManager::CreateNewVirtualFolder(HString folderFullPath)
{
	HString vpvf = folderFullPath;
	FileSystem::ClearPathSeparation(vpvf);
	auto vfit = _assets_vf.find(vpvf);
	if (vfit == _assets_vf.end())
	{
		VirtualFolder newVF;
		newVF.FolderName = FileSystem::GetFileName(folderFullPath);
		newVF.Path = folderFullPath;
		_assets_vf.emplace(vpvf, newVF);
	}
}

void ContentManager::UpdateToolTips(AssetInfoBase* asset)
{
	SetToolTip(asset,"资产名", asset->displayName);
	SetToolTip(asset, "资产类型", GetAssetTypeString(asset->type));
}

void ContentManager::SetToolTip(AssetInfoBase* asset, HString name, HString value)
{
	auto it = asset->toolTips.find(name);
	if (it != asset->toolTips.end())
	{
		it->second = ":" + value;
	}
	else
	{
		asset->toolTips.emplace(name, ":" + value);
	}
}

#endif

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
		//虚拟路径Virtual Path
		info->virtualPath = i.attribute(L"VPath").as_string();
		//FileSystem::FixUpPath(info->virtualPath);
		info->virtualFilePath = FileSystem::Append(info->virtualPath , info->displayName + "." + info->suffix);
		info->virtualFilePath.Replace("\\","/");

		//FileSystem::FixUpPath(info->virtualFilePath);
		info->repository = repositoryName;

		info->toolTips.reserve(20);
		UpdateToolTips(info.get());

		//读取引用关系
		info->depTemps.clear();
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
			if (HString("Dep").IsSame(j.name()))
			{
				info->depTemps.push_back(newTemp);
			}
			else
			{
				info->refTemps.push_back(newTemp);
			}
		}

		//Get memory
		{
			//1
			auto ait = _assets[(uint32_t)type].find(info->guid);
			if (ait == _assets[(uint32_t)type].end())
			{
				_assets[(uint32_t)type].emplace(info->guid, info);
			}
			else
			{
				_assets[(uint32_t)type][info->guid] = info;
			}
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
				it->second.emplace(info->guid, info);
			}
			//3
			HString vpvf = info->virtualPath;
			FileSystem::ClearPathSeparation(vpvf);
			auto vfit = _assets_vf.find(vpvf);
			if (vfit == _assets_vf.end())
			{
				VirtualFolder newVF;
				newVF.assets.emplace(info->guid, info);
				newVF.FolderName = FileSystem::GetFileName(info->virtualPath);
				newVF.Path = info->virtualPath;
				_assets_vf.emplace(vpvf, newVF);
			}
			else
			{
				vfit->second.assets.emplace(info->guid, info);
			}
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

inline const std::unordered_map<HGUID, std::shared_ptr<AssetInfoBase>> ContentManager::GetAssetsByVirtualFolder(HString virtualFolder) const
{
	FileSystem::ClearPathSeparation(virtualFolder);
	auto it = _assets_vf.find(virtualFolder);
	if (it != _assets_vf.end())
	{
		return it->second.assets;
	}
	return std::unordered_map<HGUID, std::shared_ptr<AssetInfoBase>>();
}

void ContentManager::UpdateAssetReference(std::weak_ptr<AssetInfoBase> info)
{
	if (!info.expired())
	{
		info.lock()->refs.clear();
		info.lock()->deps.clear();
		for (auto i : info.lock()->refTemps)
		{
			auto it = _assets[(uint32_t)i.type].find(i.guid);
			if (it != _assets[(uint32_t)i.type].end())
			{
				info.lock()->refs.push_back(it->second);
			}
		}
		for (auto i : info.lock()->depTemps)
		{
			auto it = _assets[(uint32_t)i.type].find(i.guid);
			if (it != _assets[(uint32_t)i.type].end())
			{
				info.lock()->deps.push_back(it->second);
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