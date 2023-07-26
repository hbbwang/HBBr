#include "Common.h"
#include "FileSystem.h"
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

HString FileSystem::GetProgramPath()
{
    return HString::GetExePathWithoutFileName();
}

HString FileSystem::GetShaderCacheAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Resource"/ "ShaderCache").c_str();
    path += "/";
    path.CorrectionPath();
    return path;
}

HString FileSystem::GetResourceAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Resource").c_str();
    path += "/";
    path.CorrectionPath();
    return path;
}

HString FileSystem::GetConfigAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Config").c_str();
    path += "/";
    path.CorrectionPath();
    return path;
}

HString FileSystem::GetShaderIncludeAbsPath()
{
    fs::path p = GetProgramPath().c_str();
    HString path = (p / "Resource" / "ShaderInclude").c_str();
    path += "/";
    path.CorrectionPath();
    return path;
}

HString FileSystem::GetRelativePath(const char* filePath)
{
    HString programPath = GetProgramPath().c_str();
    programPath.CorrectionPath();
    HString path = fs::path(filePath).c_str();
    path.CorrectionPath();
    if (path.Contains(programPath))
    {
        path.Remove(0, programPath.Length());
        path = "." + HString::GetSeparate() +  path;
        return path;
    }
    return filePath;
}

std::vector<FileEntry> FileSystem::GetFilesBySuffix(const char* path, const char* suffix)
{
    if (!fs::exists(path))
    {
        MessageOut("Get Files By Suffix Failed.Path is not exists.",false,true);
        return {};
    }
    std::vector<FileEntry> result;
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_regular_file()) //如果是文件
        {
            HString ext = entry.path().extension().c_str();
            ext.Remove(".");
            if (ext.IsSame(suffix))
            {
                FileEntry en = {};
                en.absPath = entry.path().c_str();
                en.relativePath = GetRelativePath(en.absPath.c_str());
                en.fileName = entry.path().filename().c_str();
                en.suffix = entry.path().extension().c_str();
                en.baseName = entry.path().stem().c_str();
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