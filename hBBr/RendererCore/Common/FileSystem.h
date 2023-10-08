#pragma once
#include "Common.h"
#include <vector>
#include <filesystem>
#include "HString.h"

enum class FileEntryType
{
	Dir = 0,
	File = 1,
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

#if __ANDROID__
extern "C" {
#endif

	class FileSystem
	{
	public:

		HBBR_API static HString GetProgramPath();
		HBBR_API static HString GetShaderCacheAbsPath();
		HBBR_API static HString GetResourceAbsPath();
		HBBR_API static HString GetContentAbsPath();
		HBBR_API static HString GetConfigAbsPath();
		/* editor only */
		HBBR_API static HString GetShaderIncludeAbsPath();
		HBBR_API static HString GetRelativePath(const char* path);
		HBBR_API static uint32_t GetPathFileNum(const char* path);
		HBBR_API static bool FileExist(const char* path);
		HBBR_API static bool IsDir(const char* path);
		HBBR_API static bool IsNormalFile(const char* path);
		HBBR_API static bool FileCopy(const char* srcFile, const char* newPath);
		HBBR_API static bool FileRemove(const char* path);
		HBBR_API static uint64_t GetFileSize(const char* path);
		static std::vector<FileEntry> GetFilesBySuffix(const char* path, const char* suffix);
		static std::vector<char>ReadBinaryFile(const char* filePath);

	};

#if __ANDROID__
}
#endif