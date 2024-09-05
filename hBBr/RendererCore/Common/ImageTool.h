#pragma once
#include "Common.h"
#include "TypeConversion.h"
#include "HString.h"
#include <algorithm>
#include "Asset/HGuid.h"
#include "VulkanManager.h"
#include <memory>

inline const char* GetTGAImageTypeString(uint8_t imageType)
{
	if (imageType == 0) return "no image data is present";
	else if (imageType == 1) return "uncompressed color-mapped image";
	else if (imageType == 2) return "uncompressed true-color image";
	else if (imageType == 3) return "uncompressed black-and-white(grayscale) image";
	else if (imageType == 9) return "run-length encoded color-mapped image";
	else if (imageType == 10) return "run-length encoded true-color image";
	else if (imageType == 11) return "run-length encoded black-and-white(grayscale) image";
	return "UnKnow";
}

//TGA文件头
struct ImageHeader
{
	uint8_t  idLength;
	uint8_t  colormapType;
	uint8_t  imageType;         //图像类型
	uint16_t colormapOrigin;
	uint16_t colormapLength;
	uint8_t  colormapDepth;
	uint16_t xOrigin;
	uint16_t yOrigin;
	uint16_t width;             //宽度
	uint16_t height;            //高度
	uint8_t  bitsPerPixel;      //每像素多少位
	uint8_t  imageDescriptor;
};

enum class ESampler
{
	LINEAR_WARP_ANISO = 0,
	POINT_WARP_ANISO = 1,
	LINEAR_CLAMP_ANISO = 2,
	POINT_CLAMP_ANISO = 3,
};

struct ImageData
{
	ImageHeader data_header;
	HString fileName;
	HString filePath;
	std::vector<unsigned char> imageData;
	std::vector<float> imageDataF;
	//float* imageDataF = nullptr;
	VkFormat texFormat;
	VkDeviceSize imageSize = 0;
	unsigned int mipLevel = 1;
	uint8_t blockSize = 0;
	bool	isCubeMap = false;
	bool	isArray = false;
};


class ImageTool
{
public:

	HBBR_API static bool SaveImage8Bit(const char* filename, int w, int h, int channels, const void* data, int jpgQuality = 80);

	HBBR_API static bool SaveImage32Bit(const char* filename, int w, int h, int channels, const float* data);

	HBBR_API static std::shared_ptr<ImageData> LoadDDSTexture(const char* filename);

	//加载非高动态范围图像，例如:png，tga，bmp，jpg等...
	HBBR_API static std::shared_ptr<ImageData> LoadImage8Bit(const char* filename, bool bsRGB = true);

	//加载高动态范围图像，例如:hdr
	HBBR_API static std::shared_ptr<ImageData> LoadImage32Bit(const char* filename);

	HBBR_API static void ImageFlipY(uint32_t w,uint32_t h, uint32_t d, void* data);

};