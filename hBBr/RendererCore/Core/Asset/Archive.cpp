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
		if (!root)
		{
			root = ArchiveTargetFile.append_child(TEXT("root"));
		}
		for (int i = 0; i < Properties.size(); i++)
		{
			auto groupNode = XMLStream::GetXMLNode(root, Properties[i].PropertyName.c_wstr());
			groupNode.set_value(Properties[i].PropertyValue.c_wstr());
		}
		ArchiveTargetFile.save_file(ArchiveTargetAbsFilePath.c_wstr());
	}
}

void Archive::InitArchive(HString assetSavePath)
{
	if (bInit)
	{
		return;
	}
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
		XMLStream::CreateXMLDocument(ArchiveTargetAbsFilePath, ArchiveTargetFile);
		ArchiveTargetFile.append_child(TEXT("root"));
	}
}

void Archive::Add(HString name, HString value)
{
	ArchiveLayout newLayout;
	newLayout.PropertyName = name;
	newLayout.PropertyValue = value;
	Properties.push_back(newLayout);
}
