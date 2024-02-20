#include "Archive.h"
#include "FileSystem.h"
//被释放的时候,自动进行存档。
Archive::~Archive()
{
	SaveArchive();
}

//也可以自主调用存档
void Archive::SaveArchive()
{
	if (bInit)
	{
		auto root = ArchiveTargetFile.child(TEXT("root"));
		for (int i = 0; i < Properties.size(); i++)
		{
			auto groupNode = XMLStream::GetXMLNode(root, Properties[i].Group.c_wstr());
			groupNode.append_child(TEXT("Item")).append_attribute(Properties[i].PropertyName.c_wstr()).set_value(Properties[i].PropertyValue.c_wstr());
		}
		ArchiveTargetFile.save_file(ArchiveTargetAbsFilePath.c_wstr());
	}
}

void Archive::InitArchive(HString assetSavePath)
{
	ArchiveTargetAbsFilePath = FileSystem::FillUpAssetPath(assetSavePath);
	bInit = false;
	if (assetSavePath.IsEmpty())
	{
		return;
	}
	bInit = true;
	if (FileSystem::FileExist(ArchiveTargetAbsFilePath))
	{
		XMLStream::LoadXML(ArchiveTargetAbsFilePath.c_wstr(), ArchiveTargetFile);
	}
	else
	{
		XMLStream::CreateXMLFile(ArchiveTargetAbsFilePath, ArchiveTargetFile);
		ArchiveTargetFile.append_child(TEXT("root"));
	}
}

void Archive::Add(HString name, HString value, HString group)
{
	ArchiveLayout newLayout;
	newLayout.Group = group;
	newLayout.PropertyName = name;
	newLayout.PropertyValue = value;
	Properties.push_back(newLayout);
}
