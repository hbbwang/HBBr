#pragma once
//基层HObject,管理对象
#include "Common.h"
#include <memory>
#include <vector>
#include <map>
#include <iostream>
#include "HString.h"
//Vulkan api
#include "vulkan/vulkan.h"

enum class SceneTextureDesc {
	SceneColor = 0,
	SceneDepth = 1,
};

class VulkanRenderer;
class PassBase;
class Texture;

class SceneTexture
{
public:
	SceneTexture(VulkanRenderer* renderer);
	~SceneTexture()
	{
		_sceneTexture.clear();
	}
	void UpdateTextures();
	std::shared_ptr<Texture> GetTexture(SceneTextureDesc desc)
	{
		auto it = _sceneTexture.find(desc);
		if (it != _sceneTexture.end())
		{
			return it->second;
		}
		return NULL;
	}
private:
	std::map<SceneTextureDesc, std::shared_ptr<Texture>> _sceneTexture;
	class VulkanRenderer* _renderer;
};

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

	__forceinline VkExtent2D GetImageSize()const {
		return _imageSize;
	}

	void Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0 , uint32_t mipLevelCount = 1);

	void TransitionImmediate(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1);

	void Resize(uint32_t width, uint32_t height);

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
	VkExtent2D _imageSize;
	VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};
