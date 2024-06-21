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
};

HString GetRendererConfig(HString Group, HString name);
void SaveRendererConfig();
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

HString GetInternationalizationText(HString Group, HString name);