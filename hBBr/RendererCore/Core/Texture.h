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

	static std::shared_ptr<Texture> CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName = "Texture", bool noMemory = false);

private:
	bool _bNoMemory = false;
	VkImage _image;
	VkImageView _imageView;
	VkDeviceMemory _imageViewMemory;
	VkFormat _format;
	VkImageUsageFlags _usageFlags;
	HString _textureName;
};

class FrameBufferTexture
{
public:

	FrameBufferTexture() {}
	~FrameBufferTexture();
	static std::shared_ptr<FrameBufferTexture> CreateFrameBuffer(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags);

	std::shared_ptr<Texture> GetBuffer();

private:
	std::vector<std::shared_ptr<Texture>> _frameBufferTextures;
};