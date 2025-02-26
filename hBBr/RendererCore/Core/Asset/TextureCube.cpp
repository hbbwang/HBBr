﻿#include "TextureCube.h"
#include "filesystem.h"
#include "ConsoleDebug.h"
#include "DDSTool.h"

TextureCube::TextureCube()
{
}

TextureCube::~TextureCube()
{
}

std::shared_ptr<TextureCube> TextureCube::CreateTextureCube(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, std::string textureName, uint32_t miplevel)
{
	std::shared_ptr<TextureCube> newTexture = std::make_shared<TextureCube>();
	newTexture->_textureName = textureName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = { width,height };
	newTexture->_imageData.isCubeMap = true;
	newTexture->_imageData.data_header.width = width;
	newTexture->_imageData.data_header.height = height;
	VulkanManager::GetManager()->CreateImage(width, height, format, usageFlags, newTexture->_image, miplevel, 6);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	newTexture->_textureMemorySize = VulkanManager::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory);
	_textureStreamingSize += newTexture->_textureMemorySize;
	VulkanManager::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView, miplevel, 6);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;
	newTexture->_bReset = true;
	if (!newTexture->_assetInfo.expired())
	{
		VulkanObjectManager::Get()->RefreshTexture(newTexture);
	}
	return newTexture;
}

std::shared_ptr<TextureCube> TextureCube::LoadAsset(HGUID guid, VkImageUsageFlags usageFlags)
{
	const auto texAssets = ContentManager::Get()->GetAssets(AssetType::TextureCube);
	std::string guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = texAssets.find(guid);
	{
		if (it == texAssets.end())
		{
			MessageOut(std::string("Can not find [" + guidStr + "] cube texture in content manager."), false, false, "255,255,0");
			return nullptr;
		}
	}
	auto dataPtr = std::static_pointer_cast<AssetInfo<TextureCube>>(it->second);

	//是否需要重新加载
	bool bReload = false;
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	else if (!dataPtr->IsAssetLoad() && dataPtr->GetSharedPtr())
	{
		bReload = true;
	}

	//获取实际路径
	std::string filePath = it->second->absFilePath;
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return nullptr;
	}
#if _DEBUG
	ConsoleDebug::print_endl("Import cubemap(dds) texture :" + filePath, "255,255,255");
#endif
	//Load dds
	DDSLoader loader(filePath.c_str());
	auto out = loader.LoadDDSToImage();
	if (out == nullptr)
	{
		return nullptr;
	}

	if (!out->isCubeMap)
	{
		ConsoleDebug::printf_endl_warning("The texture asset is not a cube map.");
		return nullptr;
	}

	std::shared_ptr<TextureCube> newTexture;
	if (!bReload)
	{
		newTexture.reset(new TextureCube);
	}
	else
	{
		//重新刷新asset
		newTexture = dataPtr->GetSharedPtr();
	}

	//Create TextureCube Object.
	uint32_t w = out->data_header.width;
	uint32_t h = out->data_header.height;
	VkFormat format = out->texFormat;

	newTexture->_assetInfo = dataPtr;
	newTexture->_textureName = dataPtr->displayName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = { w, h };
	newTexture->_imageData = *out;
	uint32_t arrayLevel = 6;

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

	//标记为需要CopyBufferToImage
	newTexture->UploadToGPU();

	newTexture->_bReset = true;
	if (!newTexture->_assetInfo.expired())
	{
		VulkanObjectManager::Get()->RefreshTexture(newTexture);
	}

	dataPtr->SetData(newTexture);
	return dataPtr->GetData();
}
#if IS_EDITOR
void TextureCube::SaveAsset(std::string path)
{

}
#endif