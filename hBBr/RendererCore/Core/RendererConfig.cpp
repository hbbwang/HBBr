#include "RendererConfig.h"
#include "Common.h"
#include "FormMain.h"
nlohmann::json RenderConfig::_renderer_json;
nlohmann::json RenderConfig::_internationalzation_json;
nlohmann::json RenderConfig::_editor_internationalzation_json;
nlohmann::json RenderConfig::_editor_json;
std::map<HString, std::map<HString, HString>> RenderConfig::_rendererConfigDatas;
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

int GetRendererConfigInt(HString Group, HString name)
{
	auto it = RenderConfig::_rendererConfigDatas.find(Group);
	if (it != RenderConfig::_rendererConfigDatas.end())
	{
		auto itt = it->second.find(name);
		if (itt != it->second.end())
		{
			return HString::ToInt(itt->second);
		}
	}
	return 0;
}

void LoadRendererConfig()
{
	if (RenderConfig::_renderer_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "renderer.json", RenderConfig::_renderer_json);
	}
	RenderConfig::_rendererConfigDatas.clear();
	for (auto& group : RenderConfig::_renderer_json.items())
	{
		HString groupName = group.key();
		auto& ggg = RenderConfig::_rendererConfigDatas[groupName];
		for (auto& value : group.value().items())
		{
			HString newName = value.key();
			HString newValue;
			if(value.value().is_string())
				newValue = value.value();
			else if (value.value().is_number_integer())
				newValue = HString::FromInt(value.value());
			else if (value.value().is_number_float())
				newValue = HString::FromFloat(value.value());
			else if (value.value().is_boolean())
				newValue = HString::FromBool(value.value());
			else if (value.value().is_number_unsigned())
				newValue = HString::FromUInt(value.value());
			ggg.emplace(newName, newValue);
		}
	}
}

void SaveRendererConfig()
{
	for (auto& i : RenderConfig::_rendererConfigDatas)
	{
		HString group = i.first;
		for (auto& b: i.second)
		{
			HString name = b.first;
			RenderConfig::_renderer_json[group.c_str()][name.c_str()] = b.second.c_str();
		}

	}
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

HString GetEditorInternationalizationText(HString Group, HString name)
{
	if (RenderConfig::_editor_internationalzation_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor_localization.json", RenderConfig::_editor_internationalzation_json);
	}
	if (!RenderConfig::_editor_internationalzation_json.is_null())
	{
		auto git = RenderConfig::_editor_internationalzation_json.find(Group.c_str());
		if (git != RenderConfig::_editor_internationalzation_json.end())
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

int GetEditorInternationalizationInt(HString Group, HString name)
{
	if (RenderConfig::_editor_internationalzation_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor_localization.json", RenderConfig::_editor_internationalzation_json);
	}
	if (!RenderConfig::_editor_internationalzation_json.is_null())
	{
		auto it = RenderConfig::_editor_internationalzation_json.find(Group.c_str());
		if (it != RenderConfig::_editor_internationalzation_json.end())
		{
			auto va_it = it.value().find(name.c_str());
			if (va_it != it.value().end())
			{
				int result = va_it.value();
				return result;
			}
		}
	}
	return 0;
}

HString GetEditorConfig(HString Group, HString name)
{
	if (RenderConfig::_editor_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor.json", RenderConfig::_editor_json);
	}
	if (!RenderConfig::_editor_json.is_null())
	{
		auto git = RenderConfig::_editor_json.find(Group.c_str());
		if (git != RenderConfig::_editor_json.end())
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

int GetEditorConfigInt(HString Group, HString name)
{
	if (RenderConfig::_editor_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor.json", RenderConfig::_editor_json);
	}
	if (!RenderConfig::_editor_json.is_null())
	{
		auto it = RenderConfig::_editor_json.find(Group.c_str());
		if (it != RenderConfig::_editor_json.end())
		{
			auto va_it = it.value().find(name.c_str());
			if (va_it != it.value().end())
			{
				int result = va_it.value();
				return result;
			}
		}
	}
	return 0;
}

void SaveInternationalizationText(HString Group, HString name)
{

}
