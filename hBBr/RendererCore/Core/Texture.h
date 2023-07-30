#pragma once
//基层HObject,管理对象
#include "Common.h"
#include <memory>
#include <vector>
#include <iostream>
#include "HString.h"
//Vulkan api
#include "vulkan/vulkan.h"

class Texture
{
	friend class VulkanManager;
public:
	Texture() {}
	Texture(bool bNoMemory) {_bNoMemory = bNoMemory;}
	~Texture();
	__forceinline VkImage GetTexture()const {
		return _image;
	}
	__forceinline VkImageView GetTextureView()const {
		return _imageView;
	}
	__forceinline VkFormat GetFormat()const {
		return _format;
	}
	__forceinline VkImageLayout GetLayout ()const {
		return _imageLayout;
	}
	__forceinline VkImageUsageFlags GetUsageFlags()const {
		return _usageFlags;
	}
	__forceinline uint32_t GetMipCount()const {
		return _mipCount;
	}
	__forceinline VkImageAspectFlags GetAspectFlags()const {
		return _imageAspectFlags;
	}

	void Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0 , uint32_t mipLevelCount = 1);

	static std::shared_ptr<Texture> CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName = "Texture", bool noMemory = false);

	HString _textureName;

private:
	bool _bNoMemory = false;
	VkImage _image;
	VkImageView _imageView;
	VkDeviceMemory _imageViewMemory;
	VkFormat _format;
	VkImageUsageFlags _usageFlags;
	VkImageAspectFlags _imageAspectFlags;
	uint32_t _mipCount = 1;
	VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};
