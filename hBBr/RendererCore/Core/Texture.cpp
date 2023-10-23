#include <ostream>
#include "Texture.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
#include "ContentManager.h"
#include "FileSystem.h"
#include "ConsoleDebug.h"
#include "DDSTool.h"
#include "ContentManager.h"
#include "XMLStream.h"
std::vector<Texture*> Texture::_upload_textures;
std::unordered_map<HString, Texture*> Texture::_system_textures;
std::unordered_map < TextureSampler, VkSampler > Texture::_samplers;
std::vector<FontTextureInfo> Texture::_fontTextureInfos;

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

void Texture::Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount, uint32_t baseArrayLayer, uint32_t layerCount)
{
	VulkanManager::GetManager()->Transition(cmdBuffer, _image, _imageAspectFlags, oldLayout, newLayout, mipLevelBegin, mipLevelCount, baseArrayLayer, layerCount);
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
	VulkanManager::GetManager()->CreateImage(w, h, format, usageFlags, newTexture->_image, newTexture->_imageData->mipLevel);
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
	info.minLod = 0;
	info.maxLod = 15;//max 16384
	//info.anisotropyEnable = VK_FALSE;
	//info.maxAnisotropy = 1.0f;
	//各项异性采样
	if (manager->GetPhysicalDeviceFeatures().samplerAnisotropy == VK_TRUE)
	{
		info.anisotropyEnable = VK_TRUE;
		info.maxAnisotropy = 16.0f;
	}
	else
	{
		info.anisotropyEnable = VK_FALSE;
		info.maxAnisotropy = 1.0f;
	}
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

	//Create BaseTexture
	auto blackTex = Texture::ImportTextureAsset(ContentManager::Get()->GetAssetGUID(FileSystem::GetContentAbsPath() + "Core/Texture/T_System_Black.dds"));
	auto normalTex = Texture::ImportTextureAsset(ContentManager::Get()->GetAssetGUID(FileSystem::GetContentAbsPath() + "Core/Texture/T_System_Normal.dds"));
	auto whiteTex = Texture::ImportTextureAsset(ContentManager::Get()->GetAssetGUID(FileSystem::GetContentAbsPath() + "Core/Texture/T_System_White.dds"));
	auto testTex = Texture::ImportTextureAsset(ContentManager::Get()->GetAssetGUID(FileSystem::GetContentAbsPath() + "Core/Texture/TestTex.dds"));
	auto fontTex = Texture::ImportTextureAsset(ContentManager::Get()->GetAssetGUID(FileSystem::GetContentAbsPath() + "Core/Texture/font.dds"));
	Texture::AddSystemTexture("Black", blackTex);
	Texture::AddSystemTexture("Normal", normalTex);
	Texture::AddSystemTexture("White", whiteTex);
	Texture::AddSystemTexture("TestTex", testTex);
	Texture::AddSystemTexture("Font", fontTex);

	//导入文字信息
	{
		HString fontDocPath = FileSystem::GetConfigAbsPath() + "Font.xml";
		pugi::xml_document fontDoc;
		fontDoc.load_file(fontDocPath.c_wstr());
		auto root = fontDoc.child(L"root");
		auto num = root.attribute(L"num").as_ullong();
		_fontTextureInfos.resize(num);
		for (auto i = root.first_child(); i != NULL; i = i.next_sibling())
		{
			FontTextureInfo info;
			info.posX = i.attribute(L"x").as_uint();
			info.posY = i.attribute(L"y").as_uint();
			info.sizeX = i.attribute(L"w").as_uint();
			info.sizeY = i.attribute(L"h").as_uint();
			_fontTextureInfos[i.attribute(L"id").as_uint()] = (info);
		}
	}
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
	//系统纹理是渲染器底层预设纹理，需要直接准备就绪
	tex->CopyBufferToTextureImmediate();
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

bool Texture::CopyBufferToTexture(VkCommandBuffer cmdbuf)
{
	const auto& manager = VulkanManager::GetManager();
	if (_imageData)
	{
		//上一帧已经复制完成了,可以删除Upload buffer 和Memory了
		if (_bUploadToGPU)
		{
			if (_uploadBuffer != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(manager->GetDevice(), _uploadBuffer, nullptr);
				_uploadBuffer = VK_NULL_HANDLE;
			}
			if (_uploadBufferMemory != VK_NULL_HANDLE)
			{
				vkFreeMemory(manager->GetDevice(), _uploadBufferMemory, nullptr);
				_uploadBufferMemory = VK_NULL_HANDLE;
			}
			return true;
		}
		else if (_imageData->imageData.size() > 0)
		{
			//Copy image data to upload memory
			uint64_t imageSize = _imageData->imageSize;

			//Cube 有6面
			int faceNum = 1;
			if (_imageData->isCubeMap)
			{
				faceNum = 6;
			}
			imageSize *= faceNum;

			//创建Buffer储存Image data
			manager->CreateBufferAndAllocateMemory(
				imageSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_uploadBuffer,
				_uploadBufferMemory);
			void* data = NULL;
			vkMapMemory(manager->GetDevice(), _uploadBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, _imageData->imageData.data(), (size_t)imageSize);
			vkUnmapMemory(manager->GetDevice(), _uploadBufferMemory);

			//从Buffer copy到Vkimage
			if (_imageData->blockSize > 0)
			{
				if (_imageData->isCubeMap)
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, _imageData->mipLevel, 0, 6);
				else
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, _imageData->mipLevel);

				std::vector<VkBufferImageCopy>region(_imageData->mipLevel * faceNum);
				int regionIndex = 0;
				uint32_t bufferOffset = 0;
				for (uint32_t face = 0; face < faceNum; face++)
				{
					uint32_t mipWidth = _imageData->data_header.width;
					uint32_t mipHeight = _imageData->data_header.height;
					for (uint32_t i = 0; i < _imageData->mipLevel; i++)
					{
						region[regionIndex] = {};
						region[regionIndex].bufferOffset = bufferOffset;
						region[regionIndex].bufferRowLength = 0;
						region[regionIndex].bufferImageHeight = 0;
						region[regionIndex].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						region[regionIndex].imageSubresource.mipLevel = i;
						region[regionIndex].imageSubresource.baseArrayLayer = face;
						region[regionIndex].imageSubresource.layerCount = 1;
						region[regionIndex].imageOffset = { 0,0,0 };
						region[regionIndex].imageExtent = { mipWidth ,mipHeight,1 };
						bufferOffset += SIZE_OF_BC((int32_t)mipWidth, (int32_t)mipHeight, _imageData->blockSize);
						if (mipWidth > 1) mipWidth /= 2;
						if (mipHeight > 1) mipHeight /= 2;
						regionIndex++;
					}
				}
				vkCmdCopyBufferToImage(cmdbuf, _uploadBuffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)region.size(), region.data());

				//复制完成后,把Layout转换到Shader read only,给shader采样使用
				if (_imageData->isCubeMap)
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, _imageData->mipLevel, 0, 6);
				else
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, _imageData->mipLevel);
			}

			//上传完成,标记
			_bUploadToGPU = true;
			//上传完成,清除图像的CPU数据
			_imageData->imageData.clear();
			return false;
		}
	}
	return false;
}

void Texture::CopyBufferToTextureImmediate()
{
	const auto& manager = VulkanManager::GetManager();
	VkCommandBuffer buf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), buf);
	manager->BeginCommandBuffer(buf, 0);
	{
		CopyBufferToTexture(buf);
	}
	manager->EndCommandBuffer(buf);
	manager->SubmitQueueImmediate({ buf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	manager->FreeCommandBuffers(manager->GetCommandPool(), { buf });
	//
	if (_bUploadToGPU)
	{
		if (_uploadBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(manager->GetDevice(), _uploadBuffer, nullptr);
			_uploadBuffer = VK_NULL_HANDLE;
		}
		if (_uploadBufferMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(manager->GetDevice(), _uploadBufferMemory, nullptr);
			_uploadBufferMemory = VK_NULL_HANDLE;
		}
	}
	auto it = std::find(_upload_textures.begin(), _upload_textures.end(), this);
	if (it != _upload_textures.end())
	{
		_upload_textures.erase(it);
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

void Texture::OutputImage(const char* outputPath, int w, int h, nvtt::Format format, void* outData)
{
	using namespace nvtt;
	Surface image;
	if (!image.setImage2D(format, w, h, outData))
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

/*--------------------字体库----------------------*/
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

struct Character {
	//对应字符
	wchar_t  font;
	//记录UV
	unsigned int u = 0;
	unsigned int v = 0;
	//字符大小
	unsigned int sizeX;
	unsigned int sizeY;
};

void Texture::CreateFontTexture(HString ttfFontPath, HString outTexturePath ,bool bOverwrite, uint32_t fontSize, uint32_t maxTextureSize)
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
		ttfFontPath = FileSystem::GetResourceAbsPath() + "Font/bahnschrift.ttf";
		ttfFontPath.CorrectionPath();
	}

	FILE* font_file = fopen(ttfFontPath.c_str(), "rb");
	fseek(font_file, 0, SEEK_END);
	size_t fontFileSize = ftell(font_file);
	fseek(font_file, 0, SEEK_SET);

	unsigned char* font_data = (unsigned char*)malloc(fontFileSize);
	fread(font_data, 1, fontFileSize, font_file);
	fclose(font_file);

	stbtt_fontinfo font;
	stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0));

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
	float scale = stbtt_ScaleForPixelHeight(&font, fontSize);

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
	unsigned char* bitmap = (unsigned char*)calloc(maxTextureSize * maxTextureSize * 4, 1);
	{
		int x = 0;
		int y = 0;
		for (wchar_t c = 32; c < 127; c++)
		{
			int c_x1, c_y1, c_x2, c_y2;
			stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

			int glyph_width, glyph_height, glyph_xoff, glyph_yoff;
			unsigned char* glyph_bitmap = stbtt_GetCodepointBitmap(
				&font, scale, scale, c, &glyph_width, &glyph_height, &glyph_xoff, &glyph_yoff);

			if (x + glyph_width > maxTextureSize)
			{
				x = 0;
				y += fontSize;
			}

			if (y + glyph_height > maxTextureSize)
			{
				break;
			}

			for (int y2 = 0; y2 < glyph_height; ++y2) {
				for (int x2 = 0; x2 < glyph_width; ++x2) {
					auto color = glyph_bitmap[y2 * glyph_width + x2];
					int x_pos = x + glyph_xoff + x2;
					int y_pos = ascent * scale + y2 + glyph_yoff;
					int index = ((y_pos + y)*maxTextureSize + x_pos) * 4;
					bitmap[index + 0] = color;
					bitmap[index + 1] = color;
					bitmap[index + 2] = color;
					bitmap[index + 3] = color;
				}
			}

			int advance, lsb;
			stbtt_GetCodepointHMetrics(&font, c, &advance, &lsb);
			Character newChar;
			newChar.font = c;
			newChar.sizeX = glyph_width;
			newChar.sizeY = glyph_height;
			newChar.u = x + lsb * scale;
			newChar.v = y + lsb * scale;
			characters.push_back(newChar);

			x += advance * scale;

			stbtt_FreeBitmap(glyph_bitmap, 0);
		}

		using namespace nvtt;
		Context context;
		context.enableCudaAcceleration(true);

		Surface image;
		image.setImage(nvtt::InputFormat_BGRA_8UB, maxTextureSize, maxTextureSize, 1, bitmap);

		OutputOptions output;
		output.setFileName(outTexturePath.c_str());
		int mipmaps = 1;
		CompressionOptions options;
		options.setFormat(Format_BC7);
		options.setQuality(Quality_Production);
		context.outputHeader(image, mipmaps, options, output);
		context.compress(image, 0, 0, options, output);
	}

	//刷新字体纹理信息
	auto assetInfo = ContentManager::Get()->ImportAssetInfo(AssetType::Texture2D, outTexturePath, outTexturePath);
	HString newName = (outTexturePath.GetFilePath() + GUIDToString(assetInfo->guid) + ".dds");
	FileSystem::FileRename(outTexturePath.c_str(), newName.c_str());
	
	//导出文字UV信息
	pugi::xml_document doc;

	//把文字信息保存到xml配置
	HString fontDocPath = FileSystem::GetConfigAbsPath() + "Font.xml";
	XMLStream::CreatesXMLFile(fontDocPath, doc);
	auto root = doc.append_child(L"root");
	root.append_attribute(L"num").set_value(((uint64_t)characters[characters.size() - 1].font) + 1);
	for (auto i : characters)
	{
		auto subNode = root.append_child(L"char");
		subNode.append_attribute(L"id").set_value((uint64_t)i.font);
		subNode.append_attribute(L"x").set_value(i.u);
		subNode.append_attribute(L"y").set_value(i.v);
		subNode.append_attribute(L"w").set_value(i.sizeX);
		subNode.append_attribute(L"h").set_value(i.sizeY);
		subNode.append_attribute(L"char").set_value(std::wstring(1, i.font).c_str());
	}
	doc.save_file(fontDocPath.c_wstr());
}
#else
void Texture::CreateFontTexture(HString ttfFontPath, HString outTexturePath, bool bOverwrite, uint32_t fontSize, uint32_t maxTextureSize)
{

}
#endif