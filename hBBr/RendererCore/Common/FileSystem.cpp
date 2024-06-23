#include "Common.h"
#include "FileSystem.h"
#include <iostream>
#include <fstream>
#include "RendererConfig.h"
#include "AndroidCommon.h"

namespace fs = std::filesystem;

HString FileSystem::_appPath;

HString FileSystem::GetProgramPath()
{
    //const char* org = "hBBr";
    //const char* app = "Game";
    //char* prefPath = SDL_GetPrefPath(org, app);
    //SDL_ShowSimpleMessageBox(0, "", prefPath, nullptr);
    //SDL_free(prefPath);

    if (_appPath.Length() <= 2)
    {
        #if _WIN32
        auto path = SDL_GetBasePath();
        char pathStr[4096];
        strcpy_s(pathStr, 4096, path);
        pathStr[strlen(path)] = '\0';
        _appPath = pathStr;
        SDL_free(path);
        #elif __ANDROID__
        //获取android/data/ [package] /files路径
        _appPath = SDL_AndroidGetExternalStoragePath();
        _appPath += "/";
        #endif
    }
    return _appPath;
}

HString FileSystem::GetShaderCacheAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Asset"/ "ShaderCache").c_str();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

HString FileSystem::GetAssetAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Asset").c_str();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

HString FileSystem::GetContentAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Asset" / "Content").c_str();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

HString FileSystem::GetConfigAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Config").c_str();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

HString FileSystem::GetWorldAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Asset" / "World").c_str();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

HString FileSystem::GetRepositoryConfigAbsPath(HString repositoryName)
{
    HString contentPath = FileSystem::GetContentAbsPath();
    HString repositoryPath = FileSystem::Append(contentPath, repositoryName);
    return FileSystem::Append(repositoryPath, ".repository");
}

HString FileSystem::GetRepositoryAbsPath(HString repositoryName)
{
    HString contentPath = FileSystem::GetContentAbsPath();
    return FileSystem::Append(contentPath, repositoryName);
}

HString FileSystem::FillUpAssetPath(HString assetPath)
{
    fs::path p = GetProgramPath().c_str();
    fs::path a = assetPath.c_str();
    p = p / a;
    p = p.lexically_normal();
    return p.c_str();
}

HString FileSystem::GetShaderIncludeAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Asset" / "ShaderInclude").c_str();
	path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

HString FileSystem::GetRelativePath(const char* filePath)
{
    HString programPath = GetProgramPath().c_str();
    FileSystem::CorrectionPath(programPath);
    HString path = fs::path(filePath).c_str();
    FileSystem::CorrectionPath(path);
    if (path.Contains(programPath))
    {
        path.Remove(0, programPath.Length());
        path = "." + HString::GetSeparate() +  path;
        return path;
    }
    return filePath;
}

HString FileSystem::GetRelativePath(HString filePath)
{
    HString programPath = GetProgramPath().c_str();
    FileSystem::CorrectionPath(programPath);
    HString path = fs::path(filePath.c_str()).c_str();
    FileSystem::CorrectionPath(path);
    if (path.Contains(programPath))
    {
        path.Remove(0, programPath.Length());
        path = "." + HString::GetSeparate() + path;
        return path;
    }
    return filePath;
}

uint32_t FileSystem::GetPathFileNum(const char* path)
{
    if (!fs::exists(path))
    {
        MessageOut(GetInternationalizationText("Renderer", "A000016"), false, true);
        return 0;
    }
    uint32_t count = 0;
    for (const auto& entry : fs::directory_iterator(path))
    {
        if(entry.exists())
            count++;
    }
    return count;
}

bool FileSystem::FileExist(const char* path)
{
    return fs::exists(path);
}

bool FileSystem::FileExist(HString path)
{
    return fs::exists(path.c_str());
}

HString FileSystem::AssetFileExist(HString path)
{
    if (!FileExist(path))
    {
        path = FileSystem::FillUpAssetPath(path);
    }
    if (!FileExist(path))
    {
        return "";
    }
    return path;
}

bool FileSystem::IsDir(HString& path)
{
    return  fs::is_directory(path.c_str());
}

bool FileSystem::CreateDir(const char* path)
{
    std::error_code error;
    auto result = fs::create_directory(path, error);;
    if (error)
    {
        MessageOut(error.message(), false, false, "255,255,0");
    }
    else
    {
        if (!result)
        {
            MessageOut(GetInternationalizationText("Renderer","A000014"), false, false, "255,255,0");
        }
    }
    return result;
}

bool FileSystem::CreateDirSymlink(const char* path, const char* linkTo)
{
    std::error_code error;
    fs::create_directory_symlink(path, linkTo, error);
    if (error)
    {
        MessageOut(error.message(), false, false, "255,255,0");
        return false;
    }
    else
    {
        return true;
    }
}

bool FileSystem::IsNormalFile(const char* path)
{
    return  fs::is_regular_file(path);
}

void FileSystem::FileCopy(const char* srcFile, const char* newPath)
{
    std::error_code error;
    fs::copy(srcFile, newPath, fs::copy_options::overwrite_existing, error);
    if (error.value())
    {
        HString copyError = "FileCopy:";
        copyError += srcFile;
        copyError += " [to] \n" ;
        copyError += newPath;
        copyError += " : \n";
        copyError += error.message().c_str();
        MessageOut(copyError, false, false, "255,255,0");
    }
}

bool FileSystem::FileRemove(const char* path)
{  
    return fs::remove(path);
}

void FileSystem::FileRename(const char* src, const char* dst)
{
    FileCopy(src, dst);
    fs::remove(src);
}

uint64_t FileSystem::GetFileSize(const char* path)
{
    return fs::file_size(path);
}

HString FileSystem::Append(HString a, HString b)
{
    fs::path pp = a.c_str();
    fs::path aa = b.c_str();
    pp = pp / aa;
    pp = pp.lexically_normal();
    return pp.c_str();
}

HString FileSystem::CorrectionPath(const char* path)
{
    //std::filesystem::path::preferred_separator
    return fs::path(path).make_preferred().c_str();
}

void FileSystem::CorrectionPath(HString& path)
{
    path = fs::path(path.c_str()).make_preferred().c_str();
}

void FileSystem::NormalizePath(HString& path)
{
    if (IsDir(path))
    {
        path = (fs::path((path + "/").c_str())).lexically_normal().c_str();
    }
    else
    {
        path = fs::path(path.c_str()).lexically_normal().c_str();
    }
}

void FileSystem::FixUpPath(HString& path)
{
    NormalizePath(path);
    CorrectionPath(path);
}

bool FileSystem::ContainsPath(HString A, HString B)
{
    NormalizePath(A);
    CorrectionPath(A);

    NormalizePath(B);
    CorrectionPath(B);

    fs::path a = A.c_str();
    fs::path b = B.c_str();

    // 使用std::mismatch算法比较两个路径的元素
    auto [a_it, b_it] = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
    
    // 如果路径A的所有元素都与路径B的元素匹配，则路径A是路径B的一部分
    return a_it == a.end();
}

HString FileSystem::GetFilePath(HString path)
{
    fs::path fs_path = fs::path(path.c_str());
    std::string result = fs_path.parent_path().string();
    result += fs::path::preferred_separator;
    return HString(result.c_str());
}

HString FileSystem::GetFileName(HString path)
{
    return fs::path(path.c_str()).filename().c_str();
}

HString FileSystem::GetBaseName(HString path)
{
    std::string fileName = fs::path(path.c_str()).filename().string();
    fileName = fileName.substr(0, fileName.rfind("."));
    return fileName.c_str();
}

HString FileSystem::GetFileExt(HString path)
{
    std::string result = fs::path(path.c_str()).extension().string();
    result = result.erase(0, 1);
    return result.c_str();
}

std::vector<FileEntry> FileSystem::GetFilesBySuffix(const char* path, const char* suffix)
{
    if (!fs::is_directory(path))
    { 
        MessageOut(GetInternationalizationText("Renderer", "A000015"),false,false);
        return {};
    }
    std::vector<FileEntry> result;
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_regular_file()) //如果是文件
        {
            HString ext = entry.path().extension().c_str();
            ext.Remove(".");
            if (ext.IsSame(suffix,false))
            {
                FileEntry en = {};
                en.absPath = entry.path().c_str();
                en.relativePath = GetRelativePath(en.absPath.c_str());
                en.fileName = entry.path().filename().c_str();
                en.suffix = entry.path().extension().c_str();
                en.baseName = entry.path().stem().c_str();
                en.type = FileEntryType::File;
                result.push_back(en);
            }
        }
        else if(entry.is_directory()) //文件夹
        {
            HString newDirPath = entry.path().c_str();
            auto children = GetFilesBySuffix(newDirPath.c_str(), suffix);
            if (children.size() > 0)
            {
                result.insert(result.end(), children.begin(), children.end());
            }
        }
    }
    return result;
}

std::vector<FileEntry> FileSystem::GetAllFilesExceptFolders(const char* path)
{
    if (!fs::is_directory(path))
    {
        MessageOut(GetInternationalizationText("Renderer", "A000015"), false, false);
        return {};
    }
    std::vector<FileEntry> result;
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_regular_file()) //如果是文件
        {
            HString ext = entry.path().extension().c_str();
            ext.Remove(".");
            {
                FileEntry en = {};
                en.absPath = entry.path().c_str();
                en.relativePath = GetRelativePath(en.absPath.c_str());
                en.fileName = entry.path().filename().c_str();
                en.suffix = entry.path().extension().c_str();
                en.baseName = entry.path().stem().c_str();
                en.type = FileEntryType::File;
                result.push_back(en);
            }
        }
        else if (entry.is_directory()) //文件夹
        {
            HString newDirPath = entry.path().c_str();
            auto children = GetAllFilesExceptFolders(newDirPath.c_str());
            if (children.size() > 0)
            {
                result.insert(result.end(), children.begin(), children.end());
            }
        }
    }
    return result;
}

std::vector<FileEntry> FileSystem::GetAllFolders(const char* path, bool currentDir)
{
    if (!fs::is_directory(path))
    {
        MessageOut(GetInternationalizationText("Renderer", "A000015"), false, false);
        return {};
    }
    std::vector<FileEntry> result;
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_directory()) //文件夹
        {
            HString ext = entry.path().extension().c_str();
            ext.Remove(".");
            {
                FileEntry en = {};
                en.absPath = entry.path().c_str();
                en.relativePath = GetRelativePath(en.absPath.c_str());
                en.fileName = entry.path().filename().c_str();
                en.suffix = entry.path().extension().c_str();
                en.baseName = entry.path().stem().c_str();
                en.type = FileEntryType::Dir;
                result.push_back(en);
            }
            //
            if (!currentDir)
            {
                HString newDirPath = entry.path().c_str();
                auto children = GetAllFolders(newDirPath.c_str(), currentDir);
                if (children.size() > 0)
                {
                    result.insert(result.end(), children.begin(), children.end());
                }
            }           
        }
    }
    return result;
}

std::vector<char> FileSystem::ReadBinaryFile(const char* filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        //throw std::runtime_error((DString("failed to open file : ") + filePath).c_str());
        MessageOut((HString("failed to open file : ") + filePath), false, true, "255,0,0");
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

void FileSystem::ClearPathSeparation(HString& path)
{
    std::string str = path.c_str();
    auto newEnd = std::remove_if(str.begin(), str.end(), [](char c) {
        return c == '/' || c == '\\';
    });
    str.erase(newEnd, str.end());
    path = str.c_str();
}
