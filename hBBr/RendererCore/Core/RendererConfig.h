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
	static nlohmann::json _renderer_json;
	static nlohmann::json _internationalzation_json;
	static nlohmann::json _editor_json;
	static nlohmann::json _editor_internationalzation_json;
	static std::map<HString, std::map<HString, HString>> _rendererConfigDatas;
};

HBBR_API HString GetRendererConfig(HString Group, HString name);

HBBR_API int GetRendererConfigInt(HString Group, HString name);

HBBR_API void LoadRendererConfig();

HBBR_API void SaveRendererConfig(); 

template<class T>
void UpdateRendererConfig(HString Group, HString name,T &value)
{
	if (RenderConfig::_renderer_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "renderer.json", RenderConfig::_renderer_json);
	}
	if (!RenderConfig::_renderer_json.is_null())
	{
		RenderConfig::_renderer_json[Group.c_str()][name.c_str()] = value;
	}
}

HBBR_API HString GetInternationalizationText(HString Group, HString name);

HBBR_API HString GetEditorInternationalizationText(HString Group, HString name);

HBBR_API int GetEditorInternationalizationInt(HString Group, HString name);

HBBR_API HString GetEditorConfig(HString Group, HString name);

HBBR_API int GetEditorConfigInt(HString Group, HString name);