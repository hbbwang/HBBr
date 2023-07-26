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
	static HString GetProgramPath();
	static HString GetShaderCacheAbsPath();
	static HString GetResourceAbsPath();
	static HString GetConfigAbsPath();
	/* editor only */
	static HString GetShaderIncludeAbsPath();

	static HString GetRelativePath(const char* path);
	static std::vector<FileEntry> GetFilesBySuffix(const char* path , const char* suffix);
	static std::vector<char>ReadBinaryFile(const char* filePath);
	
};