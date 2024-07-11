﻿#include <ostream>
#include "Texture2D.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
#include "ContentManager.h"
#include "FileSystem.h"
#include "ConsoleDebug.h"
#include "DDSTool.h"
#include "ContentManager.h"
#include "Serializable.h"
#include "RendererConfig.h"

std::vector<Texture2D*> Texture2D::_upload_textures;
std::unordered_map<HString, Texture2D*> Texture2D::_system_textures;
std::unordered_map<TextureSampler, std::vector<VkSampler>>Texture2D::_samplers;
std::unordered_map<wchar_t, FontTextureInfo> Texture2D::_fontTextureInfos;
std::shared_ptr<Texture2D>Texture2D::_fontTexture;
uint64_t Texture2D::_textureStreamingSize = 0;
uint64_t Texture2D::_maxTextureStreamingSize = (uint64_t)4 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024; //4 GB

Texture2D::~Texture2D()
{
	auto manager = VulkanManager::GetManager();
	if (_imageViewMemory != VK_NULL_HANDLE)
	{
		manager->FreeBufferMemory(_imageViewMemory);
		_textureStreamingSize = std::max((uint64_t)0, _textureStreamingSize - _textureMemorySize);
	}
	manager->DestroyImageView(_imageView);
	manager->DestroyImage(_image);
}

void Texture2D::Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount, uint32_t baseArrayLayer, uint32_t layerCount)
{
	VulkanManager::GetManager()->Transition(cmdBuffer, _image, _imageAspectFlags, oldLayout, newLayout, mipLevelBegin, mipLevelCount, baseArrayLayer, layerCount);
	_imageLayout = newLayout;
}

void Texture2D::TransitionImmediate(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount, uint32_t baseArrayLayer, uint32_t layerCount)
{
	auto manager = VulkanManager::GetManager();
	VkCommandBuffer cmdbuf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), cmdbuf);
	manager->BeginCommandBuffer(cmdbuf);
	{
		Transition(cmdbuf, oldLayout, newLayout, mipLevelBegin, mipLevelCount, baseArrayLayer, layerCount);
	}
	manager->EndCommandBuffer(cmdbuf);
	manager->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(manager->GetGraphicsQueue());
	manager->FreeCommandBuffer(manager->GetCommandPool(), cmdbuf);
}

void Texture2D::Resize(uint32_t width, uint32_t height)
{
	auto manager = VulkanManager::GetManager();
	if (_image == VK_NULL_HANDLE)
		return;

	if (_imageViewMemory != VK_NULL_HANDLE)
	{
		manager->FreeBufferMemory(_imageViewMemory);
		_textureStreamingSize = std::max((uint64_t)0, _textureStreamingSize - _textureMemorySize);
	}
	manager->DestroyImageView(_imageView);
	manager->DestroyImage(_image);
	//
	manager->CreateImage(width, height, _format, _usageFlags, _image);

	_textureMemorySize = manager->CreateImageMemory(_image, _imageViewMemory);
	_textureStreamingSize += _textureMemorySize;

	if (_textureStreamingSize > _maxTextureStreamingSize)
	{
		MessageOut((HString("Max texture streaming size is ") + HString::FromSize_t(_maxTextureStreamingSize) + ", but current texture streaming size is  " + HString::FromSize_t(_textureStreamingSize)), false, false, "255,255,0");
	}

	manager->CreateImageView(_image, _format, _imageAspectFlags, _imageView);
	//
	VkCommandBuffer cmdbuf;
	manager ->AllocateCommandBuffer(manager ->GetCommandPool(), cmdbuf);
	manager->BeginCommandBuffer(cmdbuf);
	{
		Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, _imageLayout);
	}
	manager->EndCommandBuffer(cmdbuf);
	manager->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(manager->GetGraphicsQueue());
	std::vector<VkCommandBuffer> bufs = { cmdbuf };
	manager->FreeCommandBuffers(manager->GetCommandPool(), bufs);

	_imageSize = { width,height };
}

std::shared_ptr<Texture2D> Texture2D::CreateTexture2D(
	uint32_t width, uint32_t height, VkFormat format, 
	VkImageUsageFlags usageFlags, HString textureName,
	uint32_t miplevel, uint32_t layerCount, VkMemoryPropertyFlags memoryPropertyFlag)
{
	std::shared_ptr<Texture2D> newTexture = std::make_shared<Texture2D>();
	newTexture->_textureName = textureName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = {width,height};
	VulkanManager::GetManager()->CreateImage(width, height, format, usageFlags, newTexture->_image, miplevel, layerCount);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	newTexture->_textureMemorySize = VulkanManager::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory, memoryPropertyFlag);
	_textureStreamingSize += newTexture->_textureMemorySize;
	VulkanManager::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView, miplevel, layerCount);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;
	return newTexture;
}

std::weak_ptr<Texture2D> Texture2D::LoadAsset(HGUID guid, VkImageUsageFlags usageFlags)
{
	const auto texAssets = ContentManager::Get()->GetAssets(AssetType::Texture2D);
	HString guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = texAssets.find(guid);
	{
		if (it == texAssets.end())
		{
			MessageOut(HString("Can not find [" + guidStr + "] texture in content manager."), false, false, "255,255,0");
			return std::weak_ptr<Texture2D>();
		}
	}
	auto dataPtr = std::static_pointer_cast<AssetInfo<Texture2D>>(it->second);

	//是否需要重新加载
	bool bReload = false;
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	else if (!dataPtr->IsAssetLoad() && dataPtr->GetMetadata())
	{
		bReload = true;
	}

	//获取实际路径
	HString filePath = it->second->absFilePath;
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return std::weak_ptr<Texture2D>();
	}
#if _DEBUG
	ConsoleDebug::print_endl("Import dds texture :" + filePath, "255,255,255");
#endif
	//Load dds
	DDSLoader loader(filePath.c_str());
	auto out = loader.LoadDDSToImage();
	if (out == nullptr)
	{
		return std::weak_ptr<Texture2D>();
	}

	std::shared_ptr<Texture2D> newTexture;
	if (!bReload)
	{
		newTexture.reset(new Texture2D);
	}
	else
	{
		//重新刷新asset
		newTexture = dataPtr->GetMetadata();
	}

	//Create Texture2D Object.
	uint32_t w = out->data_header.width;
	uint32_t h = out->data_header.height;
	VkFormat format = out->texFormat;

	newTexture->_assetInfo = dataPtr;
	newTexture->_textureName = dataPtr->displayName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = { w, h };
	newTexture->_imageData = *out;
	uint32_t arrayLevel = (newTexture->_imageData.isCubeMap) ? 6 : 1;
	VulkanManager::GetManager()->CreateImage(w, h, format, usageFlags, newTexture->_image, newTexture->_imageData.mipLevel, arrayLevel);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

	newTexture->_textureMemorySize = VulkanManager::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory);
	_textureStreamingSize += newTexture->_textureMemorySize;

	VulkanManager::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView, newTexture->_imageData.mipLevel, arrayLevel);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;

	dataPtr->SetData(std::move(newTexture));

	//标记为需要CopyBufferToImage
	_upload_textures.push_back(dataPtr->GetData().lock().get());

	return dataPtr->GetData();
}

void Texture2D::SaveAsset(HString path)
{
}

void Texture2D::GlobalInitialize()
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
	info.minLod = -100; 
	info.maxLod = VK_LOD_CLAMP_NONE;
	info.compareEnable = false;
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

	_samplers.emplace(TextureSampler_Linear_Wrap, std::vector<VkSampler>());
	_samplers.emplace(TextureSampler_Linear_Mirror, std::vector<VkSampler>());
	_samplers.emplace(TextureSampler_Linear_Clamp, std::vector<VkSampler>());
	_samplers.emplace(TextureSampler_Linear_Border, std::vector<VkSampler>());
	_samplers.emplace(TextureSampler_Nearest_Wrap, std::vector<VkSampler>());
	_samplers.emplace(TextureSampler_Nearest_Mirror, std::vector<VkSampler>());
	_samplers.emplace(TextureSampler_Nearest_Clamp, std::vector<VkSampler>());
	_samplers.emplace(TextureSampler_Nearest_Border, std::vector<VkSampler>());

	//mip level
	for (int t = 0; t < 8; t++)
	{
		_samplers.emplace((TextureSampler)t, std::vector<VkSampler>());
		for (int i = 0; i < 16; i++)
		{
			if ((TextureSampler)t == TextureSampler_Linear_Wrap)
				manager->CreateSampler(sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, (float)i, 16);
			else if ((TextureSampler)t == TextureSampler_Linear_Mirror)
				manager->CreateSampler(sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, (float)i, 16);
			else if ((TextureSampler)t == TextureSampler_Linear_Clamp)
				manager->CreateSampler(sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, (float)i, 16);
			else if ((TextureSampler)t == TextureSampler_Linear_Border)
				manager->CreateSampler(sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, (float)i, 16);
			else if ((TextureSampler)t == TextureSampler_Nearest_Wrap)
				manager->CreateSampler(sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, (float)i, 16);
			else if ((TextureSampler)t == TextureSampler_Nearest_Mirror)
				manager->CreateSampler(sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, (float)i, 16);
			else if ((TextureSampler)t == TextureSampler_Nearest_Clamp)
				manager->CreateSampler(sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, (float)i, 16);
			else if ((TextureSampler)t == TextureSampler_Nearest_Border)
				manager->CreateSampler(sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, (float)i, 16);
			_samplers[(TextureSampler)t].push_back(std::move(sampler));
		}
	}

	//Create BaseTexture
	auto uvGridTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("69056105-8b40-a3b2-65d4-95ebf2fe28fb"));
	auto blackTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("9eb473e5-6ef2-d6a8-9a31-8f4b26eb9f12"));
	auto normalTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("fa3e3e63-872f-99e3-a126-6ce5b8fedfae"));
	auto whiteTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("38c42242-404c-b372-fac8-d0441f2100f8"));
	auto testTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("eb8ac147-e469-f5a3-c48a-5daec8880f1f"));
	if(!uvGridTex.expired())
		Texture2D::AddSystemTexture("UVGrid", uvGridTex.lock().get());
	if (!whiteTex.expired())
		Texture2D::AddSystemTexture("White", whiteTex.lock().get());
	if (!blackTex.expired())
		Texture2D::AddSystemTexture("Black", blackTex.lock().get());
	if (!normalTex.expired())
		Texture2D::AddSystemTexture("Normal", normalTex.lock().get());
	if (!testTex.expired())
		Texture2D::AddSystemTexture("TestTex", testTex.lock().get());

	//导入文字纹理
	{
		HString fontTexturePath = GetRendererConfig("Default", "FontTexture");
		fontTexturePath = FileSystem::GetRelativePath(fontTexturePath.c_str());
		fontTexturePath = FileSystem::GetProgramPath() + fontTexturePath;
		FileSystem::CorrectionPath(fontTexturePath);

		std::shared_ptr<ImageData> imageData;

		if (fontTexturePath.GetSuffix().IsSame("png", false))
		{
			imageData = ImageTool::ReadPngImage(fontTexturePath.c_str());
		}
		else if (fontTexturePath.GetSuffix().IsSame("tga", false))
		{
			imageData = ImageTool::ReadTgaImage(fontTexturePath.c_str());
		}
		else if (fontTexturePath.GetSuffix().IsSame("dds", false))
		{
			imageData = ImageTool::ReadDDSImage(fontTexturePath.c_str());
		}
		else
		{
			MessageOut("Load Font Texture2D Failed!!!", false, true, "255,0,0");
		}
		if (!imageData)
		{
			MessageOut("Load Font Texture2D Failed!!Font image data is null.", false, true, "255,0,0");
		}
		_fontTexture = CreateTexture2D(imageData->data_header.width, imageData->data_header.height, imageData->texFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, "FontTexture");
		_fontTexture->_imageData = *imageData;
		//上传到GPU,并储存一份指针到System Texture2D
		AddSystemTexture("Font", _fontTexture.get());
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
		for (auto &c : char_json.items())
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

void Texture2D::GlobalUpdate()
{
}

void Texture2D::GlobalRelease()
{
	const auto& manager = VulkanManager::GetManager();
	for (auto i : _samplers)
	{
		for (auto& t : i.second)
		{
			vkDestroySampler(manager->GetDevice(), t, nullptr);
		}
		i.second.clear();
	}
	_samplers.clear();
	_fontTexture.reset();
}

void Texture2D::AddSystemTexture(HString tag, Texture2D* tex)
{
	//系统纹理是渲染器底层预设纹理，需要直接准备就绪
	tex->CopyBufferToTextureImmediate();
	_system_textures.emplace(tag, tex);
}

Texture2D* Texture2D::GetSystemTexture(HString tag)
{
	//_system_textures[tag];
	auto it = _system_textures.find(tag);
	if (it != _system_textures.end())
	{
		return it->second;
	}
	return _system_textures.begin()->second;
}

bool Texture2D::CopyBufferToTexture(VkCommandBuffer cmdbuf)
{
	const auto& manager = VulkanManager::GetManager();
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
		else if (_imageData.imageData.size() > 0 || _imageData.imageDataF.size() > 0)
		{
			//Copy image data to upload memory
			uint64_t imageSize = _imageData.imageSize;

			//Cube 有6面
			uint32_t faceNum = 1;
			if (_imageData.isCubeMap)
			{
				faceNum = 6;
			}
			imageSize *= faceNum;
			manager->CreateBufferAndAllocateMemory(
				_textureMemorySize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_uploadBuffer,
				_uploadBufferMemory);
			//创建Buffer储存Image data
			if (_imageData.imageData.size() > 0)
			{
				//char 只有一个字节，所以数组大小和imageSize一致
				void* data = nullptr;
				vkMapMemory(manager->GetDevice(), _uploadBufferMemory, 0, _textureMemorySize, 0, &data);
				memcpy(data, _imageData.imageData.data(), _imageData.imageData.size());
			}
			else if (_imageData.imageDataF.size() > 0)
			{
				//float 有4个字节，所以size = 数组大小 * sizeof(float)
				void* data = nullptr;
				vkMapMemory(manager->GetDevice(), _uploadBufferMemory, 0, _textureMemorySize, 0, &data);
				memcpy(data, _imageData.imageDataF.data(), (size_t)_imageData.imageSize);
			}
			vkUnmapMemory(manager->GetDevice(), _uploadBufferMemory);

			//从Buffer copy到Vkimage
			//if (_imageData->blockSize > 0)
			{
				if (_imageData.isCubeMap)
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, _imageData.mipLevel, 0, 6);
				else
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, _imageData.mipLevel);

				std::vector<VkBufferImageCopy>region(_imageData.mipLevel * faceNum);
				int regionIndex = 0;
				uint32_t bufferOffset = 0;
				for (uint32_t face = 0; face < faceNum; face++)
				{
					uint32_t mipWidth = _imageData.data_header.width;
					uint32_t mipHeight = _imageData.data_header.height;
					for (uint32_t i = 0; i < _imageData.mipLevel; i++)
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
						if (_imageData.blockSize > 0)
							bufferOffset += SIZE_OF_BC((int32_t)mipWidth, (int32_t)mipHeight, _imageData.blockSize);
						else
							bufferOffset += mipWidth * mipHeight;
						if (mipWidth > 1) mipWidth /= 2;
						if (mipHeight > 1) mipHeight /= 2;
						regionIndex++;
					}
				}
				vkCmdCopyBufferToImage(cmdbuf, _uploadBuffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)region.size(), region.data());

				//复制完成后,把Layout转换到Shader read only,给shader采样使用
				if (_imageData.isCubeMap)
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, _imageData.mipLevel, 0, 6);
				else
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, _imageData.mipLevel);
			}

			//上传完成,标记
			_bUploadToGPU = true;
			//上传完成,清除图像的CPU数据
			_imageData.imageData.clear();
			_imageData.imageDataF.clear();
			return false;
		}
	}
	return false;
}

void Texture2D::CopyBufferToTextureImmediate()
{
	const auto& manager = VulkanManager::GetManager();
	manager->DeviceWaitIdle();
	VkCommandBuffer buf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), buf);
	manager->BeginCommandBuffer(buf, 0);
	{
		CopyBufferToTexture(buf);
	}
	manager->EndCommandBuffer(buf);
	manager->SubmitQueueImmediate({ buf });
	manager->DeviceWaitIdle();
	manager->FreeCommandBuffer(manager->GetCommandPool(), buf);
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

bool Texture2D::CopyTextureToBuffer(VkCommandBuffer cmdbuf, Buffer* buffer, VkDeviceSize offset)
{
	VkBufferImageCopy copy = {};
	copy.bufferOffset = offset;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;
	copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.imageOffset = { 0, 0, 0 };
	copy.imageExtent = _imageSize;
	copy.imageExtent.depth = 1;
	vkCmdCopyImageToBuffer(cmdbuf, _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->GetBuffer(), 1, &copy);
	return true;
}

void Texture2D::CopyTextureToBufferImmediate(Buffer* buffer, VkDeviceSize offset)
{
	const auto& manager = VulkanManager::GetManager();
	VkCommandBuffer buf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), buf);
	manager->BeginCommandBuffer(buf, 0);
	{
		CopyTextureToBuffer(buf, buffer, offset);
	}
	manager->EndCommandBuffer(buf);
	manager->SubmitQueueImmediate({ buf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	manager->FreeCommandBuffer(manager->GetCommandPool(), buf);
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

void Texture2D::CompressionImage2D(const char* imagePath, const char* outputDDS, bool bGenerateMips, nvtt::Format format, bool bGenerateNormalMap, bool bAutoFormat)
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

/*
void Texture2D::CompressionImageCube(const char* imagePath, const char* outputDDS, bool bGenerateMips)
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

void Texture2D::DecompressionImage2D(const char* ddsPath, const char* outputPath, nvtt::Surface* outData, int32_t newWidth, int32_t newHeight, int32_t newDepth)
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

void Texture2D::DecompressionImageCube(const char* ddsPath, const char* outputPath, nvtt::Surface* outData, int32_t newWidth, int32_t newHeight, int32_t newDepth)
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

void Texture2D::OutputImage(const char* outputPath, int w, int h, nvtt::Format format, void* outData)
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

void Texture2D::GetImageDataFromCompressionData(const char* ddsPath, nvtt::Surface* outData)
{
	using namespace nvtt;
	SurfaceSet images;
	images.loadDDS(ddsPath);
	if (outData != nullptr)
	{
		*outData = images.GetSurface(0, 0);
	}
}

#pragma endregion NVTT

/*--------------------字体库----------------------*/


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

#if 1
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"
void GetFontCharacter(
	stbtt_fontinfo& font ,int& start_codepoint , int& end_codepoint , 
	std::vector<Character>& characters ,float scale,int ascent, int& x, int& y, 
	uint32_t& fontSize, uint32_t& maxTextureSize, std::vector<float> &atlas_data)
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
		characters.push_back(newChar);
		/* 调整x */
		x += (int)fontWidth ;

		stbtt_FreeBitmap(glyph_bitmap, 0);
	}
}

void Texture2D::CreateFontTexture(HString ttfFontPath, HString outTexturePath, bool bOverwrite, uint32_t fontSize, uint32_t maxTextureSize)
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
#if 1
	std::vector <float> atlas_data(maxTextureSize * maxTextureSize);
	memset(atlas_data.data(), 0, maxTextureSize * maxTextureSize );
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
		image.setImage(nvtt::InputFormat_R_32F, maxTextureSize, maxTextureSize, 1, atlas_data.data());
		if (outTexturePath.GetSuffix().IsSame("dds", false))
		{
			OutputOptions output;
			output.setFileName(outTexturePath.c_str());
			CompressionOptions options;
			options.setFormat(Format_BC7);
			options.setQuality(Quality_Highest);
			context.outputHeader(image, 1, options, output);
			context.compress(image, 0, 0, options, output);
		}
		else
		{
			image.save(outTexturePath.c_str(), false);
		}

		//释放
		free(font_buffer);
	}

#endif

	////刷新字体纹理信息
	//auto assetInfo = ContentManager::Get()->ImportAssetInfo(AssetType::Texture2D, outTexturePath, outTexturePath);
	//HString newName = (outTexturePath.GetFilePath() + GUIDToString(assetInfo->guid) + ".dds");
	//FileSystem::FileRename(outTexturePath.c_str(), newName.c_str());
	
	ConsoleDebug::print_endl(L"Create Font atlas texture finish.Save font info...生成完毕,正在导出文字UV信息...");

	//导出文字UV信息
	nlohmann::json json;

	//把文字信息保存到json配置
	HString fontDocPath = GetRendererConfig("Default", "FontConfig") ;
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
#else

#include "msdfgen.h"
void Texture2D::CreateFontTexture(HString ttfFontPath, HString outTexturePath, bool bOverwrite, uint32_t fontSize, uint32_t maxTextureSize)
{
	msdfgen::Bitmap<float, 3>msdf(maxTextureSize, maxTextureSize);
	msdfgen::Shape shape;
	//msdfgen::generateMSDF(msdf, );
}

#endif

#else
void Texture2D::CreateFontTexture(HString ttfFontPath, HString outTexturePath, bool bOverwrite, uint32_t fontSize, uint32_t maxTextureSize)
{

}
#endif