﻿#include "Serializable.h"
#include "FileSystem.h"
#include <fstream>
#include "ConsoleDebug.h"

bool Serializable::SaveJson(nlohmann::json& json, HString path)
{
    // 将 JSON 对象写入文件
    std::ofstream file(path.c_str(), std::ios::out | std::ios::trunc);
    if (file.is_open())
    {
        file << std::setw(4) << json << std::endl;
        file.close();
        //ConsoleDebug::print_endl("JSON data has been written to file.");
        return true;
    }
    else
    {
        ConsoleDebug::print_endl("Failed to write JSON data..", "255,0,0");
        return false;
    }
}

std::string gbk_to_utf8(const std::string& in) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::wstring wide_str = conv.from_bytes(in);
    return conv.to_bytes(wide_str);
}

bool Serializable::LoadJson(HString path, nlohmann::json& json)
{
    std::ifstream file(path.c_str());
    if (file)
    {
#if  _DEBUG
        try
        {
#endif
            //std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file >> json;
#if  _DEBUG
        }
        catch (const nlohmann::json::parse_error& e)
        {
            //ConsoleDebug::print_endl(HString("Failed to read JSON data..") + e.what(), "255,0,0");
            MessageOut(HString(HString("Failed to read JSON data : \n") + path  + "\n"+ e.what()), true, false, "255,0,0");
            return false;
        }
#endif
        file.close();
        ConsoleDebug::print_endl("JSON data has been read to file.");
        return true;
    }
    else
    {
        MessageOut(HString(HString("Failed to open JSON file : \n") + path ), false, false, "255,0,0");
        return false;
    }
}

void Serializable::SaveJson(HString path)
{
    SaveJson(_json, path);
}

bool Serializable::LoadJson(HString path)
{
    return LoadJson(path,_json);
}

void to_json(nlohmann::json& j, const HString& s) 
{
	j = s.c_strC();
}

void from_json(const nlohmann::json& j, HString& s)
{
	s = (j.get<std::string>()).c_str();
}

void to_json(nlohmann::json& j, const HGUID& s)
{
    j = s.str();
}

void from_json(const nlohmann::json& j, HGUID& s)
{
    std::string guidStr = (j.get<std::string>()).c_str();
    s = HGUID(guidStr.c_str());
}

void to_json(nlohmann::json& j, const glm::vec2& v)
{
    j["x"] = v.x;
    j["y"] = v.y;
}

void from_json(const nlohmann::json& j, glm::vec2& v)
{
    v.x = j["x"];
    v.y = j["y"];
}

void to_json(nlohmann::json& j, const glm::vec3& v)
{
    j["x"] = v.x;
    j["y"] = v.y;
    j["z"] = v.z;
}

void from_json(const nlohmann::json& j, glm::vec3& v)
{
    v.x = j["x"];
    v.y = j["y"];
    v.z = j["z"];
}

void to_json(nlohmann::json& j, const glm::vec4& v)
{
    j["x"] = v.x;
    j["y"] = v.y;
    j["z"] = v.z;
    j["w"] = v.w;
}

void from_json(const nlohmann::json& j, glm::vec4& v)
{
    v.x = j["x"];
    v.y = j["y"];
    v.z = j["z"];
    v.w = j["w"];
}
