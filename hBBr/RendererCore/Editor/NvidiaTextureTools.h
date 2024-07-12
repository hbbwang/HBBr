#pragma once

#include "Common.h"
#include "HString.h"
#include <vector>

#if IS_EDITOR
#include "nvtt/include/nvtt/nvtt.h"
#pragma comment(lib,"nvtt/lib/x64-v142/nvtt30106.lib")

class NVTT
{
public:
	HBBR_API static void CompressionImage2D(const char* imagePath, const char* outputDDS, bool bGenerateMips, nvtt::Format format, bool bGenerateNormalMap, bool bAutoFormat = false);
	HBBR_API static void DecompressionImage2D(const char* ddsPath, const char* outputPath, nvtt::Surface* outData = nullptr, int32_t newWidth = -1, int32_t newHeight = -1, int32_t newDepth = -1);
	HBBR_API static void DecompressionImageCube(const char* ddsPath, const char* outputPath, nvtt::Surface* outData = nullptr, int32_t newWidth = -1, int32_t newHeight = -1, int32_t newDepth = -1);
	HBBR_API static void OutputImage(const char* outputPath, int w, int h, nvtt::Format format, void* outData);
	HBBR_API static void GetImageDataFromCompressionData(const char* ddsPath, nvtt::Surface* outData);
};

#endif