#pragma once
#include "Common.h"
#include "HString.h"
#include <map>
#include <memory>
#include "Serializable.h"
#include "FileSystem.h"

class RenderConfig
{
public:

	HBBR_API HBBR_INLINE static nlohmann::json& GetRendererConfig()
	{
		return _renderer_json;
	}

	HBBR_API HBBR_INLINE static nlohmann::json& GetInternationalzationConfig()
	{
		return _internationalzation_json;
	}

	HBBR_API HBBR_INLINE static nlohmann::json& GetEditorConfig()
	{
		return _editor_json;
	}

	HBBR_API HBBR_INLINE static nlohmann::json& GetEditorInternationalzationConfig()
	{
		return _editor_internationalzation_json;
	}

	static nlohmann::json _renderer_json;
	static nlohmann::json _internationalzation_json;
	static nlohmann::json _editor_json;
	static nlohmann::json _editor_internationalzation_json;
};

HBBR_API HString GetRendererConfig(HString Group, HString name);

HBBR_API int GetRendererConfigInt(HString Group, HString name);

HBBR_API void SaveRendererConfig(); 

//template<class T>
//void UpdateRendererConfig(HString Group, HString name,T &value)
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

HBBR_API HString GetInternationalizationText(HString Group, HString name);

HBBR_API HString GetEditorInternationalizationText(HString Group, HString name);

HBBR_API int GetEditorInternationalizationInt(HString Group, HString name);

HBBR_API HString GetEditorConfig(HString Group, HString name);

HBBR_API int GetEditorConfigInt(HString Group, HString name);