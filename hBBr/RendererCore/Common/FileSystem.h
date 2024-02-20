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
		HBBR_API static HString GetAssetAbsPath();
		HBBR_API static HString GetContentAbsPath();
		HBBR_API static HString GetConfigAbsPath();
		HBBR_API static HString GetWorldAbsPath();
		//Fill up asset path (Asset/...)
		HBBR_API static HString FillUpAssetPath(HString assetPath);
		/* editor only */
		HBBR_API static HString GetShaderIncludeAbsPath();
		//
		HBBR_API static HString GetRelativePath(const char* path);
		HBBR_API static uint32_t GetPathFileNum(const char* path);
		HBBR_API static bool FileExist(const char* path);
		HBBR_API static bool FileExist(HString path);
		HBBR_API static HString AssetFileExist(HString path);
		HBBR_API static bool IsDir(const char* path);
		HBBR_API static bool IsDir(HString& path);
		HBBR_API static bool CreateDir(const char* path);
		HBBR_API static bool CreateDirSymlink(const char* createPath, const char* linkTo);
		HBBR_API static bool IsNormalFile(const char* path);
		HBBR_API static void FileCopy(const char* srcFile, const char* newPath);
		HBBR_API static bool FileRemove(const char* path);
		HBBR_API static void FileRename(const char* src , const char* dst);
		HBBR_API static uint64_t GetFileSize(const char* path);
		HBBR_API static HString Append(HString a,HString b);
		//本地化路径上的斜杠
		HBBR_API static HString CorrectionPath(const char* path);
		//本地化路径上的斜杠
		HBBR_API static void CorrectionPath(HString& path);
		//消除多余的分隔符和其他元素
		HBBR_API static void NormalizePath(HString& path);
		HBBR_API static void FixUpPath(HString& path);
		//路径A是否是路径B的一部分 (B内包含了A)
		HBBR_API static bool ContainsPath(HString A, HString B);
		HBBR_API static HString GetFilePath(HString path);
		HBBR_API static HString GetFileName(HString path);
		HBBR_API static HString GetBaseName(HString path);
		HBBR_API static HString GetFileExt(HString path);
		HBBR_API static std::vector<FileEntry> GetFilesBySuffix(const char* path, const char* suffix);
		//Get all files except folders
		HBBR_API static std::vector<FileEntry> GetAllFilesExceptFolders(const char* path);
		HBBR_API static std::vector<char>ReadBinaryFile(const char* filePath);
		static HString _appPath;
	};

#if __ANDROID__
}
#endif