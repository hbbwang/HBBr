#pragma once
#include "Common.h"
#include "StringTools.h"
#include <map>
#include <memory>
#include "Serializable.h"
#include "FileSystem.h"

class RenderConfig
{
public:

	HBBR_API HBBR_INLINE static nlohmann::json& GetRendererConfig()
	{
		static nlohmann::json _renderer_json;
		return _renderer_json;
	}

	HBBR_API HBBR_INLINE static nlohmann::json& GetInternationalzationConfig()
	{
		static nlohmann::json _internationalzation_json;
		return _internationalzation_json;
	}

	HBBR_API HBBR_INLINE static nlohmann::json& GetEditorConfig()
	{
		static nlohmann::json _editor_json;
		return _editor_json;
	}

	HBBR_API HBBR_INLINE static nlohmann::json& GetEditorInternationalzationConfig()
	{
		static nlohmann::json _editor_internationalzation_json;
		return _editor_internationalzation_json;
	}

};

HBBR_API std::string GetRendererConfig(std::string Group, std::string name);

HBBR_API int GetRendererConfigInt(std::string Group, std::string name);

HBBR_API void SaveRendererConfig(); 

//template<class T>
//void UpdateRendererConfig(std::string Group, std::string name,T &value)
//{
//	if (RenderConfig::GetRendererConfig().is_null())
//	{
//		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "renderer.json", RenderConfig::GetRendererConfig());
//	}
//	if (!RenderConfig::GetRendererConfig().is_null())
//	{
//		RenderConfig::GetRendererConfig()[Group.c_str()][name.c_str()] = value;
//	}
//}

HBBR_API std::string GetInternationalizationText(std::string Group, std::string name);

HBBR_API std::string GetEditorInternationalizationText(std::string Group, std::string name);

HBBR_API int GetEditorInternationalizationInt(std::string Group, std::string name);

HBBR_API std::string GetEditorConfig(std::string Group, std::string name);

HBBR_API int GetEditorConfigInt(std::string Group, std::string name);