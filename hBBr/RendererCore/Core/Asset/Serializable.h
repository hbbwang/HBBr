#pragma once
//Json序列化接口类
#include "Common.h"
#include "HGuid.h"
#include "HString.h"
#include <vector>
#include <any>
#include <map>
#include "ThirdParty/nlohmann/json.hpp"
class Serializable {
public:
    virtual ~Serializable() = default;

    HBBR_API virtual nlohmann::json ToJson()  = 0;
    HBBR_API virtual void FromJson() = 0;
    HBBR_API virtual void SaveJson(HString path);
    HBBR_API virtual bool LoadJson(HString path);
    HBBR_API static bool SaveJson(nlohmann::json& json, HString path);
    HBBR_API static bool LoadJson(HString path, nlohmann::json& json);
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

HBBR_API std::string gbk_to_utf8(const std::string& in);