#include "ImageTool.h"
#include <fstream>
#include <istream>

#ifdef _WIN32
#include <direct.h>
#endif

#include "ConsoleDebug.h"
#include "DDSTool.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#ifdef _WIN32
#else
#define memcpy_s(dst,dstSize,src,srcSize) memcpy(dst,src,dstSize)
#endif

HBBR_API bool ImageTool::SaveImage(const char* filename, int w, int h, int channels, const void* data, int jpgQuality)
{
	HString savePath = filename;
	HString suffix = savePath.GetSuffix();
	if (suffix.IsSame("png", false))
	{
		int stride_bytes = w * channels;
		return stbi_write_png(filename, w, h, channels, data, stride_bytes);
	}
	else if(suffix.IsSame("tga", false))
	{
		return stbi_write_tga(filename, w, h, channels, data);
	}
	else if (suffix.IsSame("jpg", false))
	{
		return stbi_write_jpg(filename, w, h, channels, data, jpgQuality);
	}
	else if (suffix.IsSame("bmp", false))
	{
		return stbi_write_bmp(filename, w, h, channels, data);
	}
	return false;
}

std::shared_ptr<ImageData> ImageTool::LoadDDSTexture(const char* filename)
{
	std::shared_ptr<ImageData> out;
	out.reset(new ImageData);
	DDSLoader loader(filename);
	out = loader.LoadDDSToImage();
	return out;
}

void GetHeaderLine(const char*& BufferPos, char Line[256])
{
	char* LinePtr = Line;
	uint32_t i;
	for (i = 0; i < 255; ++i)
	{
		if (*BufferPos == 0 || *BufferPos == 10 || *BufferPos == 13)
		{
			++BufferPos;
			break;
		}
		*LinePtr++ = *BufferPos++;
	}
	Line[i] = 0;
}

std::shared_ptr<ImageData> ImageTool::LoadImage8Bit(const char* filename, bool bsRGB)
{
	int width, height, channels;
	uint8_t* data = stbi_load(filename, &width, &height, &channels, 0);
	if (data)
	{
		size_t size = width * height * 4;
		std::vector<uint8_t> imageData(size);

		//3通道转4通道
		if (channels == 3)
		{
			for (int i = 0; i < width * height; ++i) {
				imageData[i * 4 + 0] = data[i * 3 + 0]; // R
				imageData[i * 4 + 1] = data[i * 3 + 1]; // G
				imageData[i * 4 + 2] = data[i * 3 + 2]; // B
				imageData[i * 4 + 3] = 1;					 // A
			}
			channels = 4;
		}
		else if (channels == 4 || channels == 1)
		{
			std::copy(data, data + size, imageData.begin());
		}
		else
		{
			stbi_image_free(data);
			ConsoleDebug::printf_endl_error("[LoadImage8Bit] hdr image channels >1 && < 3");
			return nullptr;
		}

		std::shared_ptr<ImageData> out;
		out.reset(new ImageData);
		out->data_header.width = width;
		out->data_header.height = height;
		out->imageData = std::move(imageData);
		out->data_header.bitsPerPixel = channels * 8;
		out->fileName = HString(filename).GetBaseName();
		out->filePath = filename;
		if (channels > 2)
		{
			if (bsRGB)
			{
				out->texFormat = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
			}
			else
			{
				out->texFormat = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
			}
		}
		else
		{
			if (bsRGB)
			{
				out->texFormat = VkFormat::VK_FORMAT_R8_UNORM;
			}
			else
			{
				//通常情况下单通道纹理我们都不会使用srgb...
				out->texFormat = VkFormat::VK_FORMAT_R8_SRGB;
			}

		}

		out->imageSize = size * sizeof(uint8_t);

		stbi_image_free(data);

		return out;
	}
	return nullptr;
}

std::shared_ptr<ImageData> ImageTool::LoadImage32Bit(const char* filename)
{
	int width, height, channels;
	float* data = stbi_loadf(filename, &width, &height, &channels, 0);
	if (data)
	{
		size_t size = width * height * 4;
		std::vector<float> imageData(size);

		//3通道转4通道
		if (channels == 3)
		{
			for (int i = 0; i < width * height; ++i) {
				imageData[i * 4 + 0] = data[i * 3 + 0]; // R
				imageData[i * 4 + 1] = data[i * 3 + 1]; // G
				imageData[i * 4 + 2] = data[i * 3 + 2]; // B
				imageData[i * 4 + 3] = 1.0f;                 // A
			}
		}
		else if(channels == 4)
		{
			std::copy(data, data + size, imageData.begin());
		}
		else
		{
			stbi_image_free(data);
			ConsoleDebug::printf_endl_error("[LoadImage32Bit] hdr image channels < 3");
			return nullptr;
		} 

		std::shared_ptr<ImageData> out;
		out.reset(new ImageData);
		out->data_header.width = width;
		out->data_header.height = height;
		out->imageDataF = std::move(imageData);
		//out->imageDataF = data;
		out->data_header.bitsPerPixel = channels * 32;
		out->fileName = HString(filename).GetBaseName();
		out->filePath = filename;
		out->texFormat = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
		out->imageSize = size * sizeof(float);

		stbi_image_free(data);

		return out;
	}
	return nullptr;
}

void ImageTool::ImageFlipY(uint32_t w, uint32_t h, uint32_t d,void *data)
{
	if (data)
	{
		uint32_t width = w;
		uint32_t height = h;
		uint32_t depth = d;
		uint32_t lineSize = width * depth;
		char* top;
		char* bottom;
		char* temp = (char*)malloc(sizeof(char) * lineSize);//一行的数据;
		for (uint32_t i = 0; i < height / 2 ; i++)
		{
			top = (char*)data + i * lineSize;
			bottom = (char*)data + (height - i - 1) * lineSize;
			memcpy(temp , bottom, lineSize);
			memcpy(bottom, top, lineSize);
			memcpy(top, temp, lineSize);
		}
		free(temp);
	}
}
