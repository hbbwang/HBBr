#pragma once
//序列化接口类
#include "XMLStream.h"
#include "HString.h"
#include <vector>

struct ArchiveLayout
{
	HString PropertyName;
	HString PropertyValue;
};

class Archive
{
public:
	//被释放的时候,自动执行SaveArchive。
	virtual ~Archive();
	//也可以手动执行
	virtual void SaveArchive();

	virtual void InitArchive(HString assetSavePath);
	void Add(HString name, HString value);
private:
	pugi::xml_document ArchiveTargetFile;
	std::vector<ArchiveLayout>Properties;
	HString ArchiveTargetAbsFilePath;
	bool bInit;
};