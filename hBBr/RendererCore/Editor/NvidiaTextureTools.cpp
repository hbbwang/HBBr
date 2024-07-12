#include "NvidiaTextureTools.h"
#include "FileSystem.h"
#include "ConsoleDebug.h"

#if IS_EDITOR



void NVTT::CompressionImage2D(const char* imagePath, const char* outputDDS, bool bGenerateMips, nvtt::Format format, bool bGenerateNormalMap, bool bAutoFormat)
{
	using namespace nvtt;

	Context context;
	context.enableCudaAcceleration(true);

	Surface image;
	bool hasAlpha = false;
	if (!image.load(imagePath, &hasAlpha))
	{
		ConsoleDebug::print_endl("NVTT:CompressionImage2D error.Load image failed.");
		return;
	}

	OutputOptions output;
	output.setFileName(outputDDS);

	int mipmaps = 1;
	if (bGenerateMips)
	{
		mipmaps = image.countMipmaps();
	}

	image.toLinear(1.0f);

	CompressionOptions options;
	options.setFormat(format);
	options.setQuality(Quality_Normal);
	if (bAutoFormat)
	{
		if (hasAlpha)
		{
			options.setFormat(Format_BC3);
		}
		else
		{
			options.setFormat(Format_BC1);
		}
	}
	if (bGenerateNormalMap || format == Format_BC3n)
	{
		options.setFormat(Format_BC3n);
		image.setNormalMap(true);
		if (bGenerateNormalMap)
		{
			image.toGreyScale(2, 1, 4, 0);
			image.copyChannel(image, 0, 3);
			image.toNormalMap(1, 0, 0, 0);
			image.packNormals();
		}
	}

	context.outputHeader(image, mipmaps, options, output);
	for (int i = 0; i < mipmaps; i++)
	{
		context.compress(image, 0, i, options, output);
		if (image.canMakeNextMipmap())
			image.buildNextMipmap(MipmapFilter_Triangle);
	}
}

void NVTT::DecompressionImage2D(const char* ddsPath, const char* outputPath, nvtt::Surface* outData, int32_t newWidth, int32_t newHeight, int32_t newDepth)
{
	using namespace nvtt;
	SurfaceSet images;
	images.loadDDS(ddsPath);
	bool needResize = false;
	int32_t width = images.GetWidth();
	int32_t height = images.GetHeight();
	int32_t depth = images.GetDepth();
	if (newWidth != -1)
	{
		width = newWidth;
		needResize = true;
	}
	if (newHeight != -1)
	{
		height = newHeight;
		needResize = true;
	}
	if (newDepth != -1)
	{
		depth = newDepth;
		needResize = true;
	}
	nvtt::Surface surface = images.GetSurface(0, 0);
	if (needResize)
		surface.resize(width, height, depth, ResizeFilter::ResizeFilter_Box);
	if (outData != nullptr)
	{
		*outData = surface;
	}
	//images.saveImage(outputPath, 0, 0);
	surface.save(outputPath);
}

void NVTT::DecompressionImageCube(const char* ddsPath, const char* outputPath, nvtt::Surface* outData, int32_t newWidth, int32_t newHeight, int32_t newDepth)
{
	using namespace nvtt;
	CubeSurface cube;
	cube.load(ddsPath, 0);
	bool needResize = false;
	if (outData != nullptr)
		*outData = cube.unfold(CubeLayout_VerticalCross);
	else
		return;
	int32_t width = outData->width();
	int32_t height = outData->height();
	int32_t depth = outData->depth();
	if (newWidth != -1)
	{
		width = newWidth;
		needResize = true;
	}
	if (newHeight != -1)
	{
		height = newHeight;
		needResize = true;
	}
	if (newDepth != -1)
	{
		depth = newDepth;
		needResize = true;
	}
	if (needResize)
		outData->resize(width, height, depth, ResizeFilter::ResizeFilter_Box);
	//images.saveImage(outputPath, 0, 0);
	outData->save(outputPath);
}

void NVTT::OutputImage(const char* outputPath, int w, int h, nvtt::Format format, void* outData)
{
	using namespace nvtt;
	Surface image;
	if (!image.setImage2D(format, w, h, outData))
	{
		ConsoleDebug::print_endl("set image 2d failed.", "255,255,0");
	}
	if (!image.save(outputPath))
	{
		ConsoleDebug::print_endl("Save output image failed", "255,255,0");
	}
}

void NVTT::GetImageDataFromCompressionData(const char* ddsPath, nvtt::Surface* outData)
{
	using namespace nvtt;
	SurfaceSet images;
	images.loadDDS(ddsPath);
	if (outData != nullptr)
	{
		*outData = images.GetSurface(0, 0);
	}
}












#endif