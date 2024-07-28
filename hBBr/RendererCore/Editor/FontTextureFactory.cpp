#include "FontTextureFactory.h"
#include "FileSystem.h"
#include "ImageTool.h"
#include "RendererConfig.h"
#include "Texture2D.h"
#include "ConsoleDebug.h"

std::unordered_map<wchar_t, FontTextureInfo> FontTextureFactory::_fontTextureInfos;
std::shared_ptr<Texture2D>FontTextureFactory::_fontTexture;

void FontTextureFactory::LoadFontTexture()
{
	//导入文字纹理
	{
		HString fontTexturePath = GetRendererConfig("Default", "FontTexture");
		fontTexturePath = FileSystem::GetRelativePath(fontTexturePath.c_str());
		fontTexturePath = FileSystem::GetProgramPath() + fontTexturePath;
		FileSystem::CorrectionPath(fontTexturePath);

		std::shared_ptr<ImageData> imageData;
		if (fontTexturePath.GetSuffix().IsSame("dds", false))
		{
			imageData = ImageTool::LoadDDSTexture(fontTexturePath.c_str());
		}
		else
		{
			imageData = ImageTool::LoadImage8Bit(fontTexturePath.c_str());
		}
		if (!imageData)
		{
			MessageOut("Load Font Texture2D Failed!!Font image data is null.", false, true, "255,0,0");
		}
		_fontTexture = Texture2D::CreateTexture2D(imageData->data_header.width, imageData->data_header.height, imageData->texFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, "FontTexture");
		_fontTexture->_imageData = *imageData;
		//上传到GPU,并储存一份指针到System Texture2D
		Texture2D::AddSystemTexture("Font", _fontTexture);
	}

	//导入文字信息
	{
		HString fontDocPath = GetRendererConfig("Default", "FontConfig");
		fontDocPath = FileSystem::GetRelativePath(fontDocPath.c_str());
		fontDocPath = FileSystem::GetProgramPath() + fontDocPath;
		FileSystem::CorrectionPath(fontDocPath);
		nlohmann::json json;
		Serializable::LoadJson(fontDocPath.c_str(), json);
		float tw = json["width"];
		float th = json["height"];
		nlohmann::json char_json = json["char"];
		for (auto& c : char_json.items())
		{
			uint64_t id = c.value()["id"];
			FontTextureInfo info;
			info.posX = (float)c.value()["x"];
			info.posY = (float)c.value()["y"];
			info.sizeX = (float)c.value()["w"];
			info.sizeY = (float)c.value()["h"];
			info.sizeOffsetX = (float)c.value()["xOffset"];
			_fontTextureInfos.emplace(std::make_pair((wchar_t)id, info));
		}
	}
}

void FontTextureFactory::ReleaseFontTexture()
{
	_fontTexture.reset();
}


#if IS_EDITOR

	#include <stdio.h>
	#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
	#include "stb_truetype.h"
	#include "NvidiaTextureTools.h"

	struct Character {
		//对应字符
		wchar_t  font;
		//字体比例，对比字体大小
		//float scale = 0;
		//记录UV
		unsigned int posX = 0;
		unsigned int posY = 0;
		//字符大小
		unsigned int sizeX;
		unsigned int sizeY;
		//单个字符的水平偏移
		unsigned int sizeOffsetX;
	};

	void GetFontCharacter(
		stbtt_fontinfo& font, int& start_codepoint, int& end_codepoint,
		std::vector<Character>& characters, float scale, int ascent, int& x, int& y,
		uint32_t& fontSize, uint32_t& maxTextureSize, std::vector<float>& atlas_data)
	{
		for (wchar_t c = start_codepoint; c <= end_codepoint; c++)
		{
			/* 获取字符的边框（边界） */
			int x0, y0, x1, y1;
			stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &x0, &y0, &x1, &y1);

			/**
				* 获取水平方向上的度量
				* advance：字宽；
				* lsb：左侧位置；
			*/
			int advance, lsb;
			stbtt_GetCodepointHMetrics(&font, c, &advance, &lsb);

			/* 调整字距 */
			float kern;
			kern = (float)stbtt_GetCodepointKernAdvance(&font, c, c + 1);
			kern = kern * scale;

			int padding = 1;
			float sdf_dis = 100;

			int glyph_width, glyph_height, glyph_xoff, glyph_yoff;
			//unsigned char* glyph_bitmap = stbtt_GetCodepointBitmap(
			//	&font, scale, scale, c, &glyph_width, &glyph_height, &glyph_xoff, &glyph_yoff);
			unsigned char* glyph_bitmap = stbtt_GetCodepointSDF(
				&font, scale, c, padding, 128, sdf_dis, &glyph_width, &glyph_height, &glyph_xoff, &glyph_yoff);
			if (!glyph_bitmap)
				continue;
			auto fontWidth = roundf(advance * scale) + roundf(kern * scale) + (float)padding + (float)padding;
			auto fontHeight = (float)ascent * scale + (float)padding + (float)padding;
			/* 计算位图的y (不同字符的高度不同） */
			if (x + (int)fontWidth > (int)maxTextureSize)
			{
				x = 0;
				y += (int)fontHeight + 1;
			}
			if (y + (int)glyph_height > (int)maxTextureSize)
			{
				return;
			}
			for (int y2 = 0; y2 < glyph_height; ++y2) {
				for (int x2 = 0; x2 < glyph_width; ++x2) {
					auto color = glyph_bitmap[y2 * glyph_width + x2];
					int x_pos = x + x2 + glyph_xoff;
					int y_pos = (int)((float)ascent * scale + (float)y2 + (float)y + (float)glyph_yoff);
					int index = ((y_pos < 0 ? 0 : y_pos) * maxTextureSize + (x_pos < 0 ? 0 : x_pos))/* * 4*/;
					if (color > 5)
						atlas_data[index] = (float)color / 255.0f;
				}
			}

			//rect.w - 字体长度得到空余的位置
			int ilen = std::max(0, (int)fontWidth - glyph_width);
			Character newChar;
			newChar.font = c;
			newChar.sizeX = (uint32_t)fontWidth - (uint32_t)padding;
			newChar.sizeY = (uint32_t)fontHeight;
			newChar.posX = std::max(x - 1, 0);
			newChar.posY = y;
			newChar.sizeOffsetX = ilen;

			if (characters.capacity() <= characters.size())
			{
				if (characters.capacity() < 200)
					characters.reserve(characters.capacity() + 100);
				else
					characters.reserve(characters.capacity() * 2);
			}
			characters.push_back(newChar);

			/* 调整x */
			x += (int)fontWidth;

			stbtt_FreeBitmap(glyph_bitmap, 0);
		}
	}

	void FontTextureFactory::CreateFontTexture(HString ttfFontPath, HString outTexturePath, bool bOverwrite, uint32_t fontSize, uint32_t maxTextureSize)
	{
		if (!bOverwrite)
		{
			if (FileSystem::FileExist(outTexturePath.c_str()))
			{
				return;
			}
		}

		if (!FileSystem::FileExist(ttfFontPath.c_str()))
		{
			ttfFontPath = FileSystem::GetConfigAbsPath() + "Theme/Fonts/arial.ttf";
			FileSystem::CorrectionPath(ttfFontPath);
			if (!FileSystem::FileExist(ttfFontPath.c_str()))
			{
				return;
			}
		}

		FILE* font_file = fopen(ttfFontPath.c_str(), "rb");
		fseek(font_file, 0, SEEK_END);
		size_t fontFileSize = ftell(font_file);
		fseek(font_file, 0, SEEK_SET);

		unsigned char* font_buffer = (unsigned char*)malloc(fontFileSize);
		fread(font_buffer, 1, fontFileSize, font_file);
		fclose(font_file);

		stbtt_fontinfo font;
		stbtt_InitFont(&font, font_buffer, stbtt_GetFontOffsetForIndex(font_buffer, 0));

		int ascent, descent, line_gap;
		stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
		float scale = stbtt_ScaleForPixelHeight(&font, (float)fontSize);

		//int text_width = 0;
		//int text_height = font_size;
		//for (wchar_t c = 32; c < 127; c++)
		//{
		//	int advance, lsb;
		//	stbtt_GetCodepointHMetrics(&font, c, &advance, &lsb);
		//	text_width += advance * scale;
		//}

		// 创建字符集
		std::vector<Character> characters;
		ConsoleDebug::print_endl(L"Create Font atlas texture...正在生成纹理图集,可能时间会比较长...莫慌...");

		std::vector <float> atlas_data(maxTextureSize * maxTextureSize);
		memset(atlas_data.data(), 0, maxTextureSize * maxTextureSize);
		{
			int x = 0;
			int y = 0;
			//基本拉丁字母（Basic Latin）：U+0020 - U+007F（包含一些特殊符号）
			int start_codepoint = 0x20;
			int end_codepoint = 0x7E;
			GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			//常用全角数字、字母和标点符号（全角ASCII字符）
			start_codepoint = 0xff01;
			end_codepoint = 0xff5e;
			GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			//常用全角括号和其他符号
			start_codepoint = 0x3000;
			end_codepoint = 0x303f;
			GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			//常用中文 CJK 统一表意符号（CJK Unified Ideographs）：U+4E00 - U+9FFF
			start_codepoint = 0x4E00;
			end_codepoint = 0x9FFF;
			GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);

			////常用中文 CJK 统一表意符号扩展 A（CJK Unified Ideographs Extension A）：U+3400 - U+4DBF
			//start_codepoint = 0x3400;
			//end_codepoint = 0x4DBF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////常用中文 CJK 统一表意符号扩展 B（CJK Unified Ideographs Extension B）：U+20000 - U+2A6DF
			//start_codepoint = 0x20000;
			//end_codepoint = 0x2A6DF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////常用中文 CJK 统一表意符号扩展 C（CJK Unified Ideographs Extension C）：U+2A700 - U+2B73F
			//start_codepoint = 0x2A700;
			//end_codepoint = 0x2B73F;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////常用中文 CJK 统一表意符号扩展 D（CJK Unified Ideographs Extension D）：U+2B740 - U+2B81F
			//start_codepoint = 0x2B740;
			//end_codepoint = 0x2B81F;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////常用中文 CJK 统一表意符号扩展 E（CJK Unified Ideographs Extension E）：U+2B820 - U+2CEAF
			//start_codepoint = 0x2B820;
			//end_codepoint = 0x2CEAF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////常用中文 CJK 统一表意符号扩展 F（CJK Unified Ideographs Extension F）：U+2CEB0 - U+2EBEF
			//start_codepoint = 0x2CEB0;
			//end_codepoint = 0x2EBEF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////常用中文 CJK 统一表意符号扩展 G（CJK Unified Ideographs Extension G）：U+30000 - U+3134F
			//start_codepoint = 0x30000;
			//end_codepoint = 0x3134F;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);

			//日文 平假名（Hiragana）：U+3040 - U+309F
			start_codepoint = 0x3040;
			end_codepoint = 0x309F;
			GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			//日文 片假名（Katakana）：U+30A0 - U+30FF
			start_codepoint = 0x30A0;
			end_codepoint = 0x30FF;
			GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);

			////U+1100 - U+11FF：Hangul Jamo（韩文字母）
			//start_codepoint = 0x1100;
			//end_codepoint = 0x11FF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////U+3130 - U+318F：Hangul Compatibility Jamo（兼容韩文字母）
			//start_codepoint = 0x3130;
			//end_codepoint = 0x318F;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////U+AC00 - U+D7AF：Hangul Syllables（韩文音节）
			//start_codepoint = 0xAC00;
			//end_codepoint = 0xD7AF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////U+A960 - U+A97F：Hangul Jamo Extended-A（扩展韩文字母A）
			//start_codepoint = 0xA960;
			//end_codepoint = 0xA97F;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////U+D7B0 - U+D7FF：Hangul Jamo Extended-B（扩展韩文字母B）
			//start_codepoint = 0xD7B0;
			//end_codepoint = 0xD7FF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);

			////拉丁字母扩展-A（Latin-1 Supplement）：U+0080 - U+00FF 
			//start_codepoint = 0x0080;
			//end_codepoint = 0x00FF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////阿拉伯数字：U+0030 - U+0039（包含在基本拉丁字母范围内）
			//start_codepoint = 0x0030;
			//end_codepoint = 0x0039;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////杂项符号（Miscellaneous Symbols）：U+2600 - U+26FF
			//start_codepoint = 0x2600;
			//end_codepoint = 0x26FF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////杂项技术（Miscellaneous Technical）：U+2300 - U+23FF
			//start_codepoint = 0x2300;
			//end_codepoint = 0x23FF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////几何形状（Geometric Shapes）：U+25A0 - U+25FF
			//start_codepoint = 0x25A0;
			//end_codepoint = 0x25FF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			//箭头（Arrows）：U+2190 - U+21FF
			start_codepoint = 0x2190;
			end_codepoint = 0x21FF;
			GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////数学运算符（Mathematical Operators）：U+2200 - U+22FF
			//start_codepoint = 0x2200;
			//end_codepoint = 0x22FF;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			//////附加数学运算符（Supplemental Mathematical Operators）：U+2A00 - U+2AFF
			////start_codepoint = 0x2A00;
			////end_codepoint = 0x2AFF;
			////GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			//////数学字母数字符号（Mathematical Alphanumeric Symbols）：U+1D400 - U+1D7FF
			////start_codepoint = 0x1D400;
			////end_codepoint = 0x1D7FF;
			////GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);
			////通用标点（General Punctuation）：U+2000 - U+206F
			//start_codepoint = 0x2000;
			//end_codepoint = 0x206F;
			//GetFontCharacter(font, start_codepoint, end_codepoint, characters, scale, ascent, x, y, fontSize, maxTextureSize, atlas_data);

			ConsoleDebug::print_endl(L"Create Font atlas texture finish.Save...生成完毕,正在保存图像...");

			//字体纹理压缩
			using namespace nvtt;
			Context context;
			context.enableCudaAcceleration(true);

			Surface image;
			if (outTexturePath.GetSuffix().IsSame("dds", false))
			{
				image.setImage(nvtt::InputFormat_R_32F, maxTextureSize, maxTextureSize, 1, atlas_data.data());
				OutputOptions output;
				output.setFileName(outTexturePath.c_str());
				CompressionOptions options;
				options.setFormat(Format_BC7);
				options.setQuality(Quality_Normal);
				context.outputHeader(image, 1, options, output);
				context.compress(image, 0, 0, options, output);
			}
			else
			{
				image.setImage(nvtt::InputFormat_R_32F, maxTextureSize, maxTextureSize, 1, atlas_data.data());
				image.save(outTexturePath.c_str(), false);
			}

			//释放
			free(font_buffer);
		}

		////刷新字体纹理信息
		//auto assetInfo = ContentManager::Get()->ImportAssetInfo(AssetType::Texture2D, outTexturePath, outTexturePath);
		//HString newName = (outTexturePath.GetFilePath() + GUIDToString(assetInfo->guid) + ".dds");
		//FileSystem::FileRename(outTexturePath.c_str(), newName.c_str());

		ConsoleDebug::print_endl(L"Create Font atlas texture finish.Save font info...生成完毕,正在导出文字UV信息...");

		//导出文字UV信息
		nlohmann::json json;

		//把文字信息保存到json配置
		HString fontDocPath = GetRendererConfig("Default", "FontConfig");
		fontDocPath = FileSystem::GetRelativePath(fontDocPath.c_str());
		fontDocPath = FileSystem::GetProgramPath() + fontDocPath;
		FileSystem::CorrectionPath(fontDocPath);
		json["num"] = characters.size();
		json["width"] = maxTextureSize;
		json["height"] = maxTextureSize;
		nlohmann::json char_json;
		for (auto i : characters)
		{
			nlohmann::json fontJson;
			fontJson["id"] = (uint64_t)i.font;
			fontJson["x"] = i.posX;
			fontJson["y"] = i.posY;

			fontJson["w"] = i.sizeX;
			fontJson["h"] = i.sizeY;
			fontJson["xOffset"] = i.sizeOffsetX;

			HString key = HString::FromSize_t(i.font);
			char_json[key.c_str()] = fontJson;
		}
		json["char"] = char_json;
		Serializable::SaveJson(json, fontDocPath);
	}

#endif
