#pragma once

#include "Common.h"
#include "HString.h"
#include <vector>

struct FontTextureInfo
{
	//Font data
	float posX;
	float posY;
	float sizeX;
	float sizeY;
	float sizeOffsetX;
};

class FontTextureFactory
{
public:
#if IS_EDITOR
	//通过ttf生成dds纹理
	static void CreateFontTexture(HString ttfFontPath, HString outTexturePath, bool bOverwrite = true, uint32_t fontSize = 48, uint32_t maxTextureSize = 256);
#endif

	static void LoadFontTexture();

	static void ReleaseFontTexture();

	HBBR_API HBBR_INLINE static class std::shared_ptr<Texture2D> GetFontTexture() {
		return _fontTexture;
	}

	HBBR_API HBBR_INLINE static FontTextureInfo* GetFontInfo(wchar_t c) {
		auto it = _fontTextureInfos.find(c);
		if (it != _fontTextureInfos.end())
		{
			return &it->second;
		}
		else
		{
			return &_fontTextureInfos[32];
		}
	}

private:
	// <wchar_t , FontTextureInfo>
	static std::unordered_map<wchar_t, FontTextureInfo> _fontTextureInfos;
	static std::shared_ptr<class Texture2D> _fontTexture;

};

