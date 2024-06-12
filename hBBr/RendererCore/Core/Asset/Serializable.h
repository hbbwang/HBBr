#pragma once
//Json序列化接口类
#include "HString.h"
#include "FileSystem.h"
#include <vector>
#include <any>
#include <map>
#include "ThirdParty/nlohmann/json.hpp"
#include "Component/GameObject.h"

class Serializable {
public:
    virtual ~Serializable() = default;

    virtual nlohmann::json ToJson()  = 0;
    virtual void FromJson() = 0;
    virtual void SaveJson(HString path);
    virtual bool LoadJson(HString path);
    static bool SaveJson(nlohmann::json& json, HString path);
    static bool LoadJson(HString path, nlohmann::json& json);
    nlohmann::json _json;
};
//自定义序列化
//HString
void to_json(nlohmann::json& j, const HString& s);
void from_json(const nlohmann::json& j, HString& s);
//GUID
void to_json(nlohmann::json& j, const HGUID& s);
void from_json(const nlohmann::json& j, HGUID& s);
//Vec3
void to_json(nlohmann::json& j, const glm::vec3& v);
void from_json(const nlohmann::json& j, glm::vec3& v);