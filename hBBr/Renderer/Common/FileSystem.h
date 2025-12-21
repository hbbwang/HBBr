#pragma once
#include "Common.h"
#include <vector>
#include <filesystem>

enum class FileEntryType
{
	Unknow = -1,
	Dir = 0,
	File = 1,
};

struct FileEntry
{
	std::string absPath = " ";
	std::string relativePath = " ";
	std::string fileName = " ";
	std::string baseName = " ";
	std::string suffix = " ";
	FileEntryType type = FileEntryType::Unknow;
};

#if __ANDROID__
extern "C" {
#endif

	class FileSystem
	{
	public:

		HBBR_API static std::string GetProgramPath();
		HBBR_API static std::string GetShaderCacheAbsPath();
		HBBR_API static std::string GetAssetAbsPath();
		HBBR_API static std::string GetContentAbsPath();
		HBBR_API static std::string GetSavedAbsPath();
		HBBR_API static std::string GetConfigAbsPath();
		HBBR_API static std::string GetWorldAbsPath();
		//获取仓库.repository 文件的绝对路径
		HBBR_API static std::string GetRepositoryConfigAbsPath(std::string repositoryName);
		//获取仓库绝对路径
		HBBR_API static std::string GetRepositoryAbsPath(std::string repositoryName);
		//Fill up asset path (Asset/...) to (C:/aa/bb/Asset/...)
		HBBR_API static std::string FillUpAssetPath(std::string assetPath);
		/* editor only */
		HBBR_API static std::string GetShaderIncludeAbsPath();
		//Remove exe path.
		HBBR_API static std::string GetRelativePath(const char* path);
		HBBR_API static std::string GetRelativePath(std::string path);
		HBBR_API static uint32_t GetPathFileNum(const char* path);
		HBBR_API static bool FileExist(const char* path);
		HBBR_API static bool FileExist(std::string path);
		HBBR_API static std::string AssetFileExist(std::string path);
		HBBR_API static bool IsDir(std::string& path);
		HBBR_API static bool CreateDir(const char* path);
		HBBR_API static bool CreateDirSymlink(const char* createPath, const char* linkTo);
		HBBR_API static bool IsNormalFile(const char* path);
		//文件复制,遇到相同目标名字的对象,则覆盖
		HBBR_API static void FileCopy(const char* srcFile, const char* newPath);
		HBBR_API static bool FileRemove(const char* path);
		HBBR_API static void FileRename(const char* src , const char* dst);
		HBBR_API static uint64_t GetFileSize(const char* path);
		HBBR_API static std::string Append(std::string a,std::string b);
		//本地化路径上的斜杠
		HBBR_API static std::string CorrectionPath(const char* path);
		//本地化路径上的斜杠
		HBBR_API static void CorrectionPath(std::string& path);
		//消除多余的分隔符和其他元素
		HBBR_API static void NormalizePath(std::string& path);
		HBBR_API static void FixUpPath(std::string& path);
		//路径A是否是路径B的一部分 (B内包含了A)
		HBBR_API static bool ContainsPath(std::string A, std::string B);
		HBBR_API static std::string GetFilePath(std::string path);
		HBBR_API static std::string GetFileName(std::string path);
		HBBR_API static std::string GetBaseName(std::string path);
		HBBR_API static std::string GetFileExt(std::string path);
		HBBR_API static std::vector<FileEntry> GetFilesBySuffix(const char* path, const char* suffix);
		//Get all files except folders
		HBBR_API static std::vector<FileEntry> GetAllFilesExceptFolders(const char* path);
		HBBR_API static std::vector<FileEntry> GetAllFolders(const char* path,bool currentDir = true);
		HBBR_API static std::vector<char>ReadBinaryFile(const char* filePath);
		//清除路径分隔符("/"和"\\")
		HBBR_API static void ClearPathSeparation(std::string& path);
	};
#if __ANDROID__
}
#endif