#include "RendererConfig.h"
#include "Common.h"

std::string GetRendererConfig(std::string Group, std::string name)
{
	if (RenderConfig::GetRendererConfig().is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "renderer.json", RenderConfig::GetRendererConfig());
	}
	if (!RenderConfig::GetRendererConfig().is_null())
	{
		auto git = RenderConfig::GetRendererConfig().find(Group.c_str());
		if (git != RenderConfig::GetRendererConfig().end())
		{
			nlohmann::json group = git.value();
			auto tit = group.find(name.c_str());
			if (tit != group.end())
			{
				std::string output = tit.value();
				return (output);
			}
		}
	}
	return "????";
}

int GetRendererConfigInt(std::string Group, std::string name)
{
	if (RenderConfig::GetRendererConfig().is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "renderer.json", RenderConfig::GetRendererConfig());
	}
	if (!RenderConfig::GetRendererConfig().is_null())
	{
		auto it = RenderConfig::GetRendererConfig().find(Group.c_str());
		if (it != RenderConfig::GetRendererConfig().end())
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
	Serializable::SaveJson(RenderConfig::GetRendererConfig(), FileSystem::GetConfigAbsPath() + "renderer.json");
}

std::string GetInternationalizationText(std::string Group, std::string name)
{
	if (RenderConfig::GetInternationalzationConfig().is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "localization.json", RenderConfig::GetInternationalzationConfig());
	}
	if (!RenderConfig::GetInternationalzationConfig().is_null())
	{
		auto git = RenderConfig::GetInternationalzationConfig().find(Group.c_str());
		if (git != RenderConfig::GetInternationalzationConfig().end())
		{
			nlohmann::json group= git.value();
			auto tit = group.find(name.c_str());
			if (tit != group.end())
			{
				std::string output = tit.value();
				return (output);
			}
		}
	}
	return "????";
}

std::string GetEditorInternationalizationText(std::string Group, std::string name)
{
	if (RenderConfig::GetEditorInternationalzationConfig().is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor_localization.json", RenderConfig::GetEditorInternationalzationConfig());
	}
	if (!RenderConfig::GetEditorInternationalzationConfig().is_null())
	{
		auto git = RenderConfig::GetEditorInternationalzationConfig().find(Group.c_str());
		if (git != RenderConfig::GetEditorInternationalzationConfig().end())
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

int GetEditorInternationalizationInt(std::string Group, std::string name)
{
	if (RenderConfig::GetEditorInternationalzationConfig().is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor_localization.json", RenderConfig::GetEditorInternationalzationConfig());
	}
	if (!RenderConfig::GetEditorInternationalzationConfig().is_null())
	{
		auto it = RenderConfig::GetEditorInternationalzationConfig().find(Group.c_str());
		if (it != RenderConfig::GetEditorInternationalzationConfig().end())
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

std::string GetEditorConfig(std::string Group, std::string name)
{
	if (RenderConfig::GetEditorConfig().is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor.json", RenderConfig::GetEditorConfig());
	}
	if (!RenderConfig::GetEditorConfig().is_null())
	{
		auto git = RenderConfig::GetEditorConfig().find(Group.c_str());
		if (git != RenderConfig::GetEditorConfig().end())
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

int GetEditorConfigInt(std::string Group, std::string name)
{
	if (RenderConfig::GetEditorConfig().is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "editor.json", RenderConfig::GetEditorConfig());
	}
	if (!RenderConfig::GetEditorConfig().is_null())
	{
		auto it = RenderConfig::GetEditorConfig().find(Group.c_str());
		if (it != RenderConfig::GetEditorConfig().end())
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

void SaveInternationalizationText(std::string Group, std::string name)
{

}
