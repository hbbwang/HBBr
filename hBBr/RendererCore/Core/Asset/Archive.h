#pragma once
//序列化接口类
#include "XMLStream.h"
#include "HString.h"
#include <vector>

struct ArchiveLayout
{
	HString Group;
	HString PropertyName;
	HString PropertyValue;
};

class Archive
{
public:
	virtual ~Archive();
	virtual void SaveArchive();
	virtual void InitArchive(HString assetSavePath);
	void Add(HString name, HString value, HString group = "Default");
private:
	pugi::xml_document ArchiveTargetFile;
	std::vector<ArchiveLayout>Properties;
	HString ArchiveTargetAbsFilePath;
	bool bInit;
};