#include "RendererConfig.h"
#include "Common.h"

nlohmann::json RenderConfig::_renderer_json;
nlohmann::json RenderConfig::_internationalzation_json;

HString GetRendererConfig(HString Group, HString name)
{
	if (RenderConfig::_renderer_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "renderer.json", RenderConfig::_renderer_json);
	}
	if (!RenderConfig::_renderer_json.is_null())
	{
		auto git = RenderConfig::_renderer_json.find(Group.c_str());
		if (git != RenderConfig::_renderer_json.end())
		{
			nlohmann::json group = git.value();
			auto tit = group.find(name.c_str());
			if (tit != group.end())
			{
				return tit.value();
			}
		}
	}
	return "????";
}

void SaveRendererConfig()
{
	Serializable::SaveJson(RenderConfig::_renderer_json, FileSystem::GetConfigAbsPath() + "renderer.json");
}

HString GetInternationalizationText(HString Group, HString name)
{
	if (RenderConfig::_internationalzation_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "localization.json", RenderConfig::_internationalzation_json);
	}
	if (!RenderConfig::_internationalzation_json.is_null())
	{
		auto git = RenderConfig::_internationalzation_json.find(Group.c_str());
		if (git != RenderConfig::_internationalzation_json.end())
		{
			nlohmann::json group= git.value();
			auto tit = group.find(name.c_str());
			if (tit != group.end())
			{
				return tit.value();
			}
		}
	}
	return "????";
}

void SaveInternationalizationText(HString Group, HString name)
{

}
