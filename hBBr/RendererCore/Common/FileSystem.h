#pragma once
#include "Common.h"
#include <vector>
#include "HString.h"

enum FileEntryType
{
	Dir = 0,
	File = 1,
	Url = 2,
};

struct FileEntry
{
	HString absPath = " ";
	HString relativePath = " ";
	HString fileName = " ";
	HString baseName = " ";
	HString suffix = " ";
	FileEntryType type = FileEntryType::File;
};

class FileSystem
{
public:
	HBBR_API static HString GetProgramPath();
	HBBR_API static HString GetShaderCacheAbsPath();
	HBBR_API static HString GetResourceAbsPath();
	HBBR_API static HString GetConfigAbsPath();
	/* editor only */
	HBBR_API static HString GetShaderIncludeAbsPath();

	HBBR_API static HString GetRelativePath(const char* path);
	static std::vector<FileEntry> GetFilesBySuffix(const char* path , const char* suffix);
	static std::vector<char>ReadBinaryFile(const char* filePath);
	
};