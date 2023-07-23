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


private :
	static std::unique_ptr<RendererConfig> _rendererConfig;
};

class RendererLauguage
{

public:

	static HString GetText(HString key);

private:

	static std::map<HString, HString> _rendererLauguageItem;

};

