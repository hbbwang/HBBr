#pragma once
#include "Common.h"
#include "TypeConversion.h"
#include "lodepng/lodepng.h"
#include "HString.h"
#include <algorithm>
#include "Asset/HGuid.h"
#include "VulkanManager.h"

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

typedef struct tagImageData
{
	ImageHeader data_header;
	HString fileName;
	HString filePath;
	std::vector<unsigned char> imageData;
	VkFormat texFormat;
	unsigned int imageSize = 0;
	unsigned int mipLevel = 1;
	HGUID	guid;
	uint8_t blockSize = 0;
	bool	isCubeMap = false;
}ImageData, * LPImage;


class ImageTool
{
public:
	HBBR_API static bool SaveTgaImage(const char* filename, ImageData* tgaData);

	HBBR_API static bool SaveTgaImage(const char* filename, uint16_t w , uint16_t h ,  uint16_t d ,  void* imageData);

	HBBR_API static bool SavePngImageGrey(const char* filename, uint16_t w, uint16_t h, void* imageData);

	HBBR_API static bool SavePngImageRGBA8(const char* filename, uint16_t w, uint16_t h, void* imageData);

	HBBR_API static bool SavePngImageRGB8(const char* filename, uint16_t w, uint16_t h, void* imageData);

	HBBR_API static ImageData* ReadDDSImage(const char* filename);

	HBBR_API static ImageData* ReadTgaImage(const char* filename);

	HBBR_API static ImageData* ReadPngImage(const char* filename);

	HBBR_API static ImageData* ReadHDRImage(const char* filename);

	HBBR_API static void ImageFlipY(uint32_t w,uint32_t h,uint32_t d, void* data);

};