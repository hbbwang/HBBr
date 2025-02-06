#pragma once
//Json序列化接口类
#include "Common.h"
#include "HGuid.h"
#include <vector>
#include <any>
#include <map>
#include "ThirdParty/nlohmann/json.hpp"
class Serializable {
public:
    virtual ~Serializable() = default;

    HBBR_API virtual nlohmann::json ToJson() {return _json;}
    HBBR_API virtual void FromJson() {}
    HBBR_API virtual void SaveJson(std::string path);
    HBBR_API virtual bool LoadJson(std::string path);
    HBBR_API static bool SaveJson(nlohmann::json& json, std::string path);
    HBBR_API static bool LoadJson(std::string path, nlohmann::json& json);
    nlohmann::json _json;
};
//自定义序列化
//std::string
HBBR_API void to_json(nlohmann::json& j, const std::string& s);
HBBR_API void from_json(const nlohmann::json& j, std::string& s);
//GUID
HBBR_API void to_json(nlohmann::json& j, const HGUID& s);
HBBR_API void from_json(const nlohmann::json& j, HGUID& s);
//Vec2
HBBR_API void to_json(nlohmann::json& j, const glm::vec2& v);
HBBR_API void from_json(const nlohmann::json& j, glm::vec2& v);
//Vec3
HBBR_API void to_json(nlohmann::json& j, const glm::vec3& v);
HBBR_API void from_json(const nlohmann::json& j, glm::vec3& v);
//Vec4
HBBR_API void to_json(nlohmann::json& j, const glm::vec4& v);
HBBR_API void from_json(const nlohmann::json& j, glm::vec4& v);

HBBR_API std::string gbk_to_utf8(const std::string& in);