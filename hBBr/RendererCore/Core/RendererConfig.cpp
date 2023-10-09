#include "RendererConfig.h"
#include "Common.h"
#include "FileSystem.h"
std::unique_ptr<RendererConfig> RendererConfig::_rendererConfig;
std::map<HString, HString> RendererLauguage::_rendererLauguageItem;

RendererConfig* RendererConfig::Get()
{
	if (!_rendererConfig)
	{
		_rendererConfig.reset(new RendererConfig);
		HString path = FileSystem::GetProgramPath() + "Config/Renderer.xml";
		path.CorrectionPath();
		auto pathChar = path.c_wstr();
		if (!XMLStream::LoadXML(pathChar, _rendererConfig->_configFile))
		{
			MessageOut("Fatal error! Load renderer config failed !!", true, true);
		}
	}
	return _rendererConfig.get();
}

HString RendererLauguage::GetText(HString key)
{
	if (_rendererLauguageItem.size() <= 0)
	{
		HString LauguageFilePath = RendererConfig::Get()->_configFile.child(L"root").child(L"BaseSetting").child(L"Language").attribute(L"path").as_string();
		LauguageFilePath = FileSystem::GetProgramPath() + LauguageFilePath;
		LauguageFilePath.CorrectionPath();
		pugi::xml_document doc;
		if (!XMLStream::LoadXML(LauguageFilePath.c_wstr(), doc))
		{
			MessageOut("Fatal error! Load lauguage file error.", true, true);
		}
		for (pugi::xml_node node = doc.child(L"root").first_child(); node != NULL; node = node.next_sibling())
		{
			HString nodeName = node.name();
			HString text = node.attribute(L"text").as_string();
			_rendererLauguageItem.insert(std::make_pair(nodeName , text));
		}
	}
	auto it = _rendererLauguageItem.find(key);
	if (it != _rendererLauguageItem.end())
	{
		return it->second;
	}
	else
	{
		return HString("???");
	}
}
