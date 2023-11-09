#include "Common.h"
#include "FileSystem.h"
#include <iostream>
#include <fstream>

#include "AndroidCommon.h"

namespace fs = std::filesystem;

HString FileSystem::_appPath;

HString FileSystem::GetProgramPath()
{
    //const char* org = "hBBr";
    //const char* app = "Game";
    //char* prefPath = SDL_GetPrefPath(org, app);
    //SDL_ShowSimpleMessageBox(0, "", prefPath, NULL);
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
    HString path = (p / "Resource"/ "ShaderCache").c_str();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

HString FileSystem::GetResourceAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Resource").c_str();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

HString FileSystem::GetContentAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Resource" / "Content").c_str();
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

HString FileSystem::GetShaderIncludeAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Resource" / "ShaderInclude").c_str();
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

uint32_t FileSystem::GetPathFileNum(const char* path)
{
    if (!fs::exists(path))
    {
        MessageOut("Get Files By Suffix Failed.Path is not exists.", false, true);
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

bool FileSystem::IsDir(const char* path)
{
    return  fs::is_directory(path);
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
        MessageOut(copyError.c_str(), false, false, "255,255,0");
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

HString FileSystem::CorrectionPath(const char* path)
{
    //std::filesystem::path::preferred_separator
    return fs::path(path).make_preferred().c_str();
}

void FileSystem::CorrectionPath(HString& path)
{
    path = fs::path(path.c_str()).make_preferred().c_str();
}

HString FileSystem::GetFilePath(HString path)
{
    std::string result = fs::path(path.c_str()).parent_path().string();
    result += fs::path::preferred_separator;
    return result.c_str();
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
        MessageOut("Get Files By Suffix Failed.Path is not exists.",false,false);
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

std::vector<char> FileSystem::ReadBinaryFile(const char* filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        //throw std::runtime_error((DString("failed to open file : ") + filePath).c_str());
        MessageOut((HString("failed to open file : ") + filePath).c_str(), false, true, "255,0,0");
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}