#include "RendererConfig.h"
#include "Common.h"
#include "FormMain.h"
nlohmann::json RenderConfig::_renderer_json;
nlohmann::json RenderConfig::_internationalzation_json;
nlohmann::json RenderConfig::_editor_internationalzation_json;
nlohmann::json RenderConfig::_editor_json;
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
	if (RenderConfig::_renderer_json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "renderer.json", RenderConfig::_renderer_json);
	}
	if (!RenderConfig::_renderer_json.is_null())
	{
		auto it = RenderConfig::_renderer_json.find(Group.c_str());
		if (it != RenderConfig::_renderer_json.end())
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
