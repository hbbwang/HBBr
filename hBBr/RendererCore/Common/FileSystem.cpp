#include "Common.h"
#include "FileSystem.h"
#include <iostream>
#include <fstream>
#include "RendererConfig.h"
#include "AndroidCommon.h"
#include "StringTools.h"

namespace fs = std::filesystem;

std::string FileSystem::GetProgramPath()
{
    //const char* org = "hBBr";
    //const char* app = "Game";
    //char* prefPath = SDL_GetPrefPath(org, app);
    //SDL_ShowSimpleMessageBox(0, "", prefPath, nullptr);
    //SDL_free(prefPath);
    #if _WIN32
        auto path = SDL_GetBasePath();
    #elif __ANDROID__
    //获取android/data/ [package] /files路径
        auto path = SDL_AndroidGetExternalStoragePath();
        path += "/";
    #endif
    return path;
}

std::string FileSystem::GetShaderCacheAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    std::string path = (p / "Asset"/ "ShaderCache").string();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

std::string FileSystem::GetAssetAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    std::string path = (p / "Asset").string();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

std::string FileSystem::GetContentAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    std::string path = (p / "Asset" / "Content").string();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

std::string FileSystem::GetSavedAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    std::string path = (p / "Asset" / "Saved").string();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

std::string FileSystem::GetConfigAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    std::string path = (p / "Config").string();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

std::string FileSystem::GetWorldAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    std::string path = (p / "Asset" / "World").string();
    path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

std::string FileSystem::GetRepositoryConfigAbsPath(std::string repositoryName)
{
    std::string contentPath = FileSystem::GetContentAbsPath();
    std::string repositoryPath = FileSystem::Append(contentPath, repositoryName);
    return FileSystem::Append(repositoryPath, ".repository");
}

std::string FileSystem::GetRepositoryAbsPath(std::string repositoryName)
{
    std::string contentPath = FileSystem::GetContentAbsPath();
    return FileSystem::Append(contentPath, repositoryName);
}

std::string FileSystem::FillUpAssetPath(std::string assetPath)
{
    fs::path p = GetProgramPath().c_str();
    fs::path a = assetPath.c_str();
    p = p / a;
    p = p.lexically_normal();
    return p.string();
}

std::string FileSystem::GetShaderIncludeAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    std::string path = (p / "Asset" / "ShaderInclude").string();
	path += "/";
    FileSystem::CorrectionPath(path);
    return path;
}

std::string FileSystem::GetRelativePath(const char* filePath)
{
    std::string programPath = GetProgramPath().c_str();
    FileSystem::CorrectionPath(programPath);
    std::string path = fs::path(filePath).string();
    FileSystem::CorrectionPath(path); 
    if (StringTool::Contains(path, programPath))
    {
        StringTool::Remove(path, 0, programPath.length());
        path = "." + StringTool::GetSeparate() +  path;
        return path;
    }
    return filePath;
}

std::string FileSystem::GetRelativePath(std::string filePath)
{
    std::string programPath = GetProgramPath().c_str();
    FileSystem::CorrectionPath(programPath);
    std::string path = fs::path(filePath.c_str()).string();
    FileSystem::CorrectionPath(path);
    if (StringTool::Contains(path, programPath))
    {
        StringTool::Remove(path, 0, programPath.length());
        path = "." + StringTool::GetSeparate() + path;
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

bool FileSystem::FileExist(std::string path)
{
    return fs::exists(path.c_str());
}

std::string FileSystem::AssetFileExist(std::string path)
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

bool FileSystem::IsDir(std::string& path)
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
            auto msg = GetInternationalizationText("Renderer", "A000014");
            MessageOut(msg, false, false, "255,255,0");
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
        std::string copyError = "FileCopy:";
        copyError += srcFile;
        copyError += " [to] \n" ;
        copyError += newPath;
        copyError += " : \n";
        copyError += (error.message()).c_str();
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

std::string FileSystem::Append(std::string a, std::string b)
{
    fs::path pp = a.c_str();
    fs::path aa = b.c_str();
    pp = pp / aa;
    pp = pp.lexically_normal();
    return pp.string();
}

std::string FileSystem::CorrectionPath(const char* path)
{
    //std::filesystem::path::preferred_separator
    return fs::path(path).make_preferred().string();
}

void FileSystem::CorrectionPath(std::string& path)
{
    path = fs::path(path.c_str()).make_preferred().string();
}

void FileSystem::NormalizePath(std::string& path)
{
    if (IsDir(path))
    {
        path = (fs::path((path + "/").c_str())).lexically_normal().string();
    }
    else
    {
        path = fs::path(path.c_str()).lexically_normal().string();
    }
}

void FileSystem::FixUpPath(std::string& path)
{
    NormalizePath(path);
    CorrectionPath(path);
}

bool FileSystem::ContainsPath(std::string A, std::string B)
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

std::string FileSystem::GetFilePath(std::string path)
{
    fs::path fs_path = fs::path(path.c_str());
    std::string result = fs_path.parent_path().string();
    result += fs::path::preferred_separator;
    return std::string(result.c_str());
}

std::string FileSystem::GetFileName(std::string path)
{
    return fs::path(path.c_str()).filename().string();
}

std::string FileSystem::GetBaseName(std::string path)
{
    std::string fileName = fs::path(path.c_str()).filename().string();
    fileName = fileName.substr(0, fileName.rfind("."));
    return fileName.c_str();
}

std::string FileSystem::GetFileExt(std::string path)
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
            std::string ext = entry.path().extension().string();
            StringTool::Remove(ext, ".");
            if(StringTool::IsEqual(ext, suffix,false))
            {
                FileEntry en = {};
                en.absPath = entry.path().string();
                en.relativePath = GetRelativePath(en.absPath.c_str());
                en.fileName = entry.path().filename().string();
                en.suffix = entry.path().extension().string();
                en.baseName = entry.path().stem().string();
                en.type = FileEntryType::File;
                const size_t vecSize = result.size();
                if (vecSize >= result.capacity())
                {
                    result.reserve(vecSize + 5);
                }
                result.push_back(en);
            }
        }
        else if(entry.is_directory()) //文件夹
        {
            std::string newDirPath = entry.path().string();
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
            std::string ext = entry.path().extension().string();
            StringTool::Remove(ext, ".");
            {
                FileEntry en = {};
                en.absPath = entry.path().string();
                en.relativePath = GetRelativePath(en.absPath.c_str());
                en.fileName = entry.path().filename().string();
                en.suffix = entry.path().extension().string();
                en.baseName = entry.path().stem().string();
                en.type = FileEntryType::File;
                const size_t vecSize = result.size();
                if (vecSize >= result.capacity())
                {
                    result.reserve(vecSize + 5);
                }
                result.push_back(en);
            }
        }
        else if (entry.is_directory()) //文件夹
        {
            std::string newDirPath = entry.path().string();
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
            std::string ext = entry.path().extension().string();
            StringTool::Remove(ext, ".");
            {
                FileEntry en = {};
                en.absPath = entry.path().string();
                en.relativePath = GetRelativePath(en.absPath.c_str());
                en.fileName = entry.path().filename().string();
                en.suffix = entry.path().extension().string();
                en.baseName = entry.path().stem().string();
                en.type = FileEntryType::Dir;
                const size_t vecSize = result.size();
                if (vecSize >= result.capacity())
                {
                    result.reserve(vecSize + 5);
                }
                result.push_back(en);
            }
            //
            if (!currentDir)
            {
                std::string newDirPath = entry.path().string();
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
        MessageOut((std::string("failed to open file : ") + filePath), false, true, "255,0,0");
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

void FileSystem::ClearPathSeparation(std::string& path)
{
    std::string str = path.c_str();
    auto newEnd = std::remove_if(str.begin(), str.end(), [](char c) {
        return c == '/' || c == '\\';
    });
    str.erase(newEnd, str.end());
    path = str.c_str();
}
