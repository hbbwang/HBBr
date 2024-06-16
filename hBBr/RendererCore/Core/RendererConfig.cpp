#include "RendererConfig.h"
#include "Common.h"
#include "FileSystem.h"
#include "Serializable.h"

std::unique_ptr<RendererConfig> RendererConfig::_rendererConfig;

RendererConfig* RendererConfig::Get()
{
	if (!_rendererConfig)
	{
		_rendererConfig.reset(new RendererConfig);
		HString path = FileSystem::GetProgramPath() + "Config/Renderer.xml";
		FileSystem::CorrectionPath(path);
		auto pathChar = path.c_wstr();
		if (!XMLStream::LoadXML(pathChar, _rendererConfig->_configFile))
		{
			MessageOut("Fatal error! Load renderer config failed !!", true, true);
		}
		_rendererConfig->_configFileRootNode = _rendererConfig->_configFile.child(TEXT("root"));
	}
	return _rendererConfig.get();
}

HString GetInternationalizationText(HString Group, HString name)
{
	static nlohmann::json json;
	if (json.is_null())
	{
		Serializable::LoadJson(FileSystem::GetConfigAbsPath() + "localization.json", json);
	}
	if (!json.is_null())
	{
		auto git = json.find(Group.c_str());
		if (git != json.end())
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
