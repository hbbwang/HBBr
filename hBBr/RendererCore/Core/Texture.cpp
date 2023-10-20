#include "Texture.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
#include "ContentManager.h"
#include "FileSystem.h"
#include "ConsoleDebug.h"
#include "DDSTool.h"

std::vector<Texture*> Texture::_upload_textures;
std::unordered_map<HString, Texture*> Texture::_system_textures;
std::unordered_map < TextureSampler, VkSampler > Texture::_samplers;

SceneTexture::SceneTexture(VulkanRenderer* renderer)
{
	_renderer = renderer;
	auto sceneColor = Texture::CreateTexture2D(1, 1, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneColor");
	auto sceneDepth = Texture::CreateTexture2D(1, 1, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "SceneDepth");
	//Transition
	VkCommandBuffer cmdbuf;
	VulkanManager::GetManager()->AllocateCommandBuffer(VulkanManager::GetManager()->GetCommandPool(), cmdbuf);
	VulkanManager::GetManager()->BeginCommandBuffer(cmdbuf);
	{
		sceneColor->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		sceneDepth->Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}
	VulkanManager::GetManager()->EndCommandBuffer(cmdbuf);
	VulkanManager::GetManager()->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), { cmdbuf });
	//
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneColor, sceneColor));
	_sceneTexture.insert(std::make_pair(SceneTextureDesc::SceneDepth, sceneDepth));
}

void SceneTexture::UpdateTextures()
{
	auto sceneDepth = _sceneTexture[SceneTextureDesc::SceneDepth];
	if (sceneDepth->GetImageSize().width != _renderer->GetSurfaceSize().width ||
		sceneDepth->GetImageSize().height != _renderer->GetSurfaceSize().height)
	{
		_sceneTexture[SceneTextureDesc::SceneDepth]->Resize(_renderer->GetSurfaceSize().width, _renderer->GetSurfaceSize().height);
	}
}

Texture::~Texture()
{
	VulkanManager::GetManager()->DestroyImageMemory(_imageViewMemory);
	VulkanManager::GetManager()->DestroyImageView(_imageView);
	VulkanManager::GetManager()->DestroyImage(_image);
}

void Texture::Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount)
{
	VulkanManager::GetManager()->Transition(cmdBuffer, _image, _imageAspectFlags, oldLayout, newLayout, mipLevelBegin, mipLevelCount);
	_imageLayout = newLayout;
}

void Texture::TransitionImmediate(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount)
{
	VkCommandBuffer cmdbuf;
	VulkanManager::GetManager()->AllocateCommandBuffer(VulkanManager::GetManager()->GetCommandPool(), cmdbuf);
	VulkanManager::GetManager()->BeginCommandBuffer(cmdbuf);
	{
		Transition(cmdbuf, oldLayout, newLayout);
	}
	VulkanManager::GetManager()->EndCommandBuffer(cmdbuf);
	VulkanManager::GetManager()->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), { cmdbuf });
}

void Texture::Resize(uint32_t width, uint32_t height)
{
	if (_image == VK_NULL_HANDLE)
		return;

	if (_imageViewMemory != VK_NULL_HANDLE)
		VulkanManager::GetManager()->FreeBufferMemory(_imageViewMemory);
	VulkanManager::GetManager()->DestroyImageView(_imageView);
	VulkanManager::GetManager()->DestroyImage(_image);
	//
	VulkanManager::GetManager()->CreateImage(width, height, _format, _usageFlags, _image);
	if (!_bNoMemory)
		VulkanManager::GetManager()->CreateImageMemory(_image, _imageViewMemory);
	VulkanManager::GetManager()->CreateImageView(_image, _format, _imageAspectFlags, _imageView);
	//
	VkCommandBuffer cmdbuf;
	VulkanManager::GetManager()->AllocateCommandBuffer(VulkanManager::GetManager()->GetCommandPool(), cmdbuf);
	VulkanManager::GetManager()->BeginCommandBuffer(cmdbuf);
	{
		Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, _imageLayout);
	}
	VulkanManager::GetManager()->EndCommandBuffer(cmdbuf);
	VulkanManager::GetManager()->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	VulkanManager::GetManager()->FreeCommandBuffers(VulkanManager::GetManager()->GetCommandPool(), { cmdbuf });

	_imageSize = { width,height };
}

std::shared_ptr<Texture> Texture::CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName, bool noMemory)
{
	std::shared_ptr<Texture> newTexture = std::make_shared<Texture>();
	newTexture->_bNoMemory = noMemory;
	newTexture->_textureName = textureName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = {width,height};
	newTexture->_sampler = _samplers[TextureSampler_Linear_Wrap];
	VulkanManager::GetManager()->CreateImage(width,height,format , usageFlags, newTexture->_image);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	if (!newTexture->_bNoMemory)
	{
		VulkanManager::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory);
	}
	VulkanManager::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;
	return newTexture;
}

Texture* Texture::ImportTextureAsset(HGUID guid, VkImageUsageFlags usageFlags)
{
	const auto texAssets = ContentManager::Get()->GetAssets(AssetType::Texture2D);
	HString guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = texAssets.find(guid);
	{
		if (it == texAssets.end())
		{
			MessageOut(HString("Can not find [" + guidStr + "] texture in content manager.").c_str(), false, false, "255,255,0");
			return NULL;
		}
	}
	auto dataPtr = reinterpret_cast<AssetInfo<Texture>*>(it->second);
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	//获取实际路径
	HString filePath = FileSystem::GetProgramPath() + it->second->relativePath + guidStr + ".dds";
	filePath.CorrectionPath();
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return NULL;
	}
#if _DEBUG
	ConsoleDebug::print_endl("Import dds texture :" + filePath, "255,255,255");
#endif
	//Load dds
	DDSLoader loader(filePath.c_str());
	ImageData* out = loader.LoadDDSToImage();

	//Create Texture Object.
	uint32_t w = out->data_header.width;
	uint32_t h = out->data_header.height;
	VkFormat format = out->texFormat;
	std::unique_ptr<Texture> newTexture = std::make_unique<Texture>();
	newTexture->_bNoMemory = false;
	newTexture->_textureName = dataPtr->name;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = { w, h };
	newTexture->_imageData = out;
	VulkanManager::GetManager()->CreateImage(w, h, format, usageFlags, newTexture->_image);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	if (!newTexture->_bNoMemory)
	{
		VulkanManager::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory);
	}
	VulkanManager::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;
	newTexture->_sampler = _samplers[TextureSampler_Linear_Wrap];

	dataPtr->SetData(std::move(newTexture));

	//标记为需要CopyBufferToImage
	_upload_textures.push_back(dataPtr->GetData());
	return dataPtr->GetData();
}

void Texture::GlobalInitialize()
{
	const auto& manager = VulkanManager::GetManager();
	//Create BaseTexture
	auto blackTex = Texture::ImportTextureAsset(HGUID("dd5644a9-ae6a-44da-8f4f-4a686b083bcc"));
	auto normalTex = Texture::ImportTextureAsset(HGUID("968561d0-8569-429c-a135-faa450b818fe"));
	auto whiteTex = Texture::ImportTextureAsset(HGUID("ca1e3bec-26d7-4c1d-91ed-da3283142736"));
	Texture::AddSystemTexture("Black", blackTex);
	Texture::AddSystemTexture("Normal", normalTex);
	Texture::AddSystemTexture("White", whiteTex);

	//Create Sampler
	VkSampler sampler = VK_NULL_HANDLE;
	VkSamplerCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.magFilter = VK_FILTER_LINEAR;
	info.minFilter = VK_FILTER_LINEAR;
	info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.minLod = -1000;
	info.maxLod = 1000;
	info.maxAnisotropy = 1.0f;
	vkCreateSampler(manager->GetDevice(), &info, nullptr, &sampler);
	_samplers.emplace(TextureSampler_Linear_Wrap, sampler);

	info.addressModeU = info.addressModeV = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	vkCreateSampler(manager->GetDevice(), &info, nullptr, &sampler);
	_samplers.emplace(TextureSampler_Linear_Mirror, sampler);

	info.addressModeU = info.addressModeV = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	vkCreateSampler(manager->GetDevice(), &info, nullptr, &sampler);
	_samplers.emplace(TextureSampler_Linear_Clamp, sampler);

	info.magFilter = info.minFilter = VK_FILTER_NEAREST;
	info.addressModeU = info.addressModeV = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	vkCreateSampler(manager->GetDevice(), &info, nullptr, &sampler);
	_samplers.emplace(TextureSampler_Nearest_Wrap, sampler);

	info.addressModeU = info.addressModeV = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	vkCreateSampler(manager->GetDevice(), &info, nullptr, &sampler);
	_samplers.emplace(TextureSampler_Nearest_Mirror, sampler);

	info.addressModeU = info.addressModeV = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	vkCreateSampler(manager->GetDevice(), &info, nullptr, &sampler);
	_samplers.emplace(TextureSampler_Nearest_Clamp, sampler);

}

void Texture::GlobalUpdate()
{
}

void Texture::GlobalRelease()
{
	const auto& manager = VulkanManager::GetManager();
	for (auto& i : _samplers)
	{
		vkDestroySampler(manager->GetDevice(), i.second, nullptr);
	}
	_samplers.clear();
}

void Texture::AddSystemTexture(HString tag, Texture* tex)
{
	_system_textures.emplace(tag, tex);
}

Texture* Texture::GetSystemTexture(HString tag)
{
	//_system_textures[tag];
	auto it = _system_textures.find(tag);
	if (it != _system_textures.end())
	{
		return it->second;
	}
	return NULL;
}

void Texture::CopyBufferToTexture(VkCommandBuffer cmdbuf, Texture* tex, std::vector<unsigned char> imageData)
{
	if (tex && imageData.size() > 0)
	{

	}
}

#ifdef IS_EDITOR

/*
#pragma region OpenCV

using namespace cv;
#define M_PI  3.14159265358979323846
// Define our six cube faces.
// 0 - 3 are side faces, clockwise order
// 4 and 5 are top and bottom, respectively
long double faceTransform[6][2] =
{
	{0, 0},
	{M_PI, 0},
	{0, -M_PI / 2}, //top
	{0, M_PI / 2},  //bottom
	{-M_PI / 2, 0},
	{M_PI / 2, 0},
};
// Map a part of the equirectangular panorama (in) to a cube face
// (face). The ID of the face is given by faceId. The desired
// width and height are given by width and height.
inline void createCubeMapFace(const Mat& in, Mat& face,
	int faceId = 0, const int width = -1,
	const int height = -1) {

	float inWidth = in.cols;
	float inHeight = in.rows;

	// Allocate map
	Mat mapx(height, width, CV_32F);
	Mat mapy(height, width, CV_32F);

	// Calculate adjacent (ak) and opposite (an) of the
	// triangle that is spanned from the sphere center
	//to our cube face.
	const float an = sin(M_PI / 4);
	const float ak = cos(M_PI / 4);

	const float ftu = faceTransform[faceId][0];
	const float ftv = faceTransform[faceId][1];

	// For each point in the target image,
	// calculate the corresponding source coordinates.
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			// Map face pixel coordinates to [-1, 1] on plane
			float nx = ((float)y / (float)height) * 2 - 1;
			float ny = ((float)x / (float)width) * 2 - 1;

			// Map [-1, 1] plane coords to [-an, an]
			// thats the coordinates in respect to a unit sphere
			// that contains our box.
			nx *= an;
			ny *= an;

			float u, v;

			// Project from plane to sphere surface.
			if (ftv == 0) {
				// Center faces
				u = atan2(nx, ak);
				v = atan2(ny * cos(u), ak);
				u += ftu;
			}
			else if (ftv > 0) {
				// Bottom face
				float d = sqrt(nx * nx + ny * ny);//dot(nx,ny)
				v = M_PI / 2 - atan2(d, ak);
				u = atan2(ny, nx);
			}
			else {
				// Top face
				float d = sqrt(nx * nx + ny * ny);//dot(nx,ny)
				v = -M_PI / 2 + atan2(d, ak);
				u = atan2(-ny, nx);
			}

			// Map from angular coordinates to [-1, 1], respectively.
			u = u / (M_PI);
			v = v / (M_PI / 2);

			// Warp around, if our coordinates are out of bounds.
			while (v < -1) {
				v += 2;
				u += 1;
			}
			while (v > 1) {
				v -= 2;
				u += 1;
			}

			while (u < -1) {
				u += 2;
			}
			while (u > 1) {
				u -= 2;
			}

			// Map from [-1, 1] to in texture space
			u = u / 2.0f + 0.5f;
			v = v / 2.0f + 0.5f;

			u = u * (inWidth - 1);
			v = v * (inHeight - 1);

			// Save the result for this pixel in map
			mapx.at<float>(x, y) = u;
			mapy.at<float>(x, y) = v;
		}
	}

	// Recreate output image if it has wrong size or type.
	if (face.cols != width || face.rows != height ||
		face.type() != in.type()) {
		face = Mat(width, height, in.type());
	}

	// Do actual resampling using OpenCV's remap
	remap(in, face, mapx, mapy, INTER_LINEAR);
}

#pragma endregion OpenCV
*/

#pragma region NVTT

void Texture::CompressionImage2D(const char* imagePath, const char* outputDDS, bool bGenerateMips, nvtt::Format format, bool bGenerateNormalMap, bool bAutoFormat)
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
	options.setQuality(Quality_Production);
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

/*
void Texture::CompressionImageCube(const char* imagePath, const char* outputDDS, bool bGenerateMips)
{
	using namespace nvtt;
	//创建上下文
	Context context;
	//启动Cuda加速
	context.enableCudaAcceleration(true);
	//纹理加载
	Surface image;
	bool hasAlpha = false;
	//使用OpenCV处理图像,从HDR全景图隐射到6面Cube
	{
		Mat hdrCube;
		//导入HDR图
		Mat in = imread(imagePath);
		int size = in.rows * in.cols;
		size /= 6;
		size = std::sqrt(size);
		for (int i = 0; i < 6; i++)
		{
			Mat newFace;
			createCubeMapFace(in, newFace, i, size, size);
			//合并每个图像，最终输出的是一张 size * (size * 6)的长条竖图
			hdrCube.push_back(newFace);
		}
		//暂时找不到方法直接把Mat的图像数据转到Surface里
		//猜测格式不一样，Mat应该是RGB,RGB...这样存的，
		//Surface则是RRRRR...GGGGG....这样的，不一样。
		//操作起来太麻烦了，就直接导出再重新导入一遍好了。
		//导出缓存图，重载载入:
		HString cachePath = imagePath;
		cachePath = cachePath.GetFilePath() + cachePath.GetBaseName() + "_Cache.hdr";
		cv::imwrite(cachePath.c_str(), hdrCube);
		if (!image.load(cachePath.c_str(), &hasAlpha))
		{
			ConsoleDebug::print("Compression Image Cube Failed.Image load failed.", "255,255,0");
			return;
		}
		//删除缓存图。
		remove(cachePath.c_str());
	}

	//cube map 需要把HDR图拆出6份，必须保证不能小于6
	if (image.height() < 6)
	{
		ConsoleDebug::print("Compression Image Cube Failed.Because HDR image height small than 6.", "255,255,0");
		return;
	}

	//转线性颜色(HDR图像一般就是非sRGB所以可以不做这个操作)
	image.toLinear(1.0);

	//导出设置
	OutputOptions output;
	output.setFileName(outputDDS);

	//设置图像HDR格式
	CompressionOptions options;
	options.setFormat(Format_BC6U);
	options.setQuality(Quality_Production);

	//把HDR纹理转换成Cube
	CubeSurface cubeImage;

	//因为是长条图，使用竖状排版拆分
	cubeImage.fold(image, CubeLayout_Column);

	//计算mipLevel
	int mipmaps = 1;
	if (bGenerateMips)
		mipmaps = cubeImage.countMipmaps();

	//设置DDS文件头
	context.outputHeader(cubeImage, mipmaps, options, output);
	//导出
	for (int f = 0; f < 6; f++)
	{
		for (int i = 0; i < mipmaps; i++)
		{
			context.compress(cubeImage.face(f), i, 1, options, output);
			if (cubeImage.face(f).canMakeNextMipmap())
				cubeImage.face(f).buildNextMipmap(MipmapFilter_Box);
		}
	}
}
*/

void Texture::DecompressionImage2D(const char* ddsPath, const char* outputPath, nvtt::Surface* outData, int32_t newWidth, int32_t newHeight, int32_t newDepth)
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
	if (outData != NULL)
	{
		*outData = surface;
	}
	//images.saveImage(outputPath, 0, 0);
	surface.save(outputPath);
}

void Texture::DecompressionImageCube(const char* ddsPath, const char* outputPath, nvtt::Surface* outData, int32_t newWidth, int32_t newHeight, int32_t newDepth)
{
	using namespace nvtt;
	CubeSurface cube;
	cube.load(ddsPath, 0);
	bool needResize = false;
	if (outData != NULL)
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

void Texture::OutputImage(const char* outputPath, int w, int h, void* outData)
{
	using namespace nvtt;
	Surface image;
	if (!image.setImage2D(Format_RGBA, w, h, outData))
	{
		ConsoleDebug::print_endl("set image 2d failed.", "255,255,0");
	}
	if (!image.save(outputPath))
	{
		ConsoleDebug::print_endl("Save output image failed.", "255,255,0");
	}
}

void Texture::GetImageDataFromCompressionData(const char* ddsPath, nvtt::Surface* outData)
{
	using namespace nvtt;
	SurfaceSet images;
	images.loadDDS(ddsPath);
	if (outData != NULL)
	{
		*outData = images.GetSurface(0, 0);
	}
}

#pragma endregion NVTT

#endif