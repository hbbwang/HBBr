#pragma once
//#define HBBR_API __attribute__((visibility("default")))
class RendererCore_Android
{
public:

	const char * getPlatformABI();

	RendererCore_Android();

	~RendererCore_Android();

	void RenderUpdate();

};

