#pragma once
//#include "Common.h"
#include "XMLStream.h"
#include "HString.h"
#include <map>
#include <memory>
class RendererConfig
{
public:

	static RendererConfig* Get();

	pugi::xml_document _configFile;
	pugi::xml_node _configFileRootNode;

private :
	static std::unique_ptr<RendererConfig> _rendererConfig;

};

HString GetInternationalizationText(HString Group, HString name);

