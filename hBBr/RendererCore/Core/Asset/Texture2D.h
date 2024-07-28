#pragma once
#include "Common.h"
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>
#include "./Asset/HGuid.h"
#include "HString.h"
#include "ImageTool.h"
#include "AssetObject.h"
#include "Buffer.h"
//Vulkan api
#include "VulkanManager.h"

enum TextureSampler
{
	TextureSampler_Linear_Wrap = 0,
	TextureSampler_Linear_Mirror = 1,
	TextureSampler_Linear_Clamp = 2,
	TextureSampler_Linear_Border = 3,
	TextureSampler_Nearest_Wrap = 4,
	TextureSampler_Nearest_Mirror = 5,
	TextureSampler_Nearest_Clamp = 6,
	TextureSampler_Nearest_Border = 7,

	TextureSampler_Max = 8,
};

class Texture2D : public AssetObject
{
	friend class VulkanManager;
	friend class ContentManager;
public:
	Texture2D() {}
	~Texture2D();
	
	HBBR_API HBBR_INLINE uint64_t GetTextureMemorySize()const {
		return _textureMemorySize;
	}
	HBBR_API HBBR_INLINE VkDeviceMemory GetTextureMemory()const {
		return _imageViewMemory;
	}
	HBBR_API HBBR_INLINE VkImage GetTexture()const {
		return _image;
	}
	HBBR_API HBBR_INLINE VkImageView GetTextureView()const {
		return _imageView;
	}
	HBBR_API HBBR_INLINE VkFormat GetFormat()const {
		return _format;
	}
	HBBR_API HBBR_INLINE VkImageLayout GetLayout ()const {
		return _imageLayout;
	}
	HBBR_API HBBR_INLINE VkImageUsageFlags GetUsageFlags()const {
		return _usageFlags;
	}
	HBBR_API HBBR_INLINE uint32_t GetMipCount()const {
		return _mipCount;
	}
	HBBR_API HBBR_INLINE VkImageAspectFlags GetAspectFlags()const {
		return _imageAspectFlags;
	}
	HBBR_API HBBR_INLINE VkExtent3D GetTextureSize()const {
		return _imageSize;
	}
	HBBR_API HBBR_INLINE static std::vector<Texture2D*>& GetUploadTextures(){
		return _upload_textures;
	}
	HBBR_API HBBR_INLINE static VkSampler GetSampler(TextureSampler sampler) {
		return _samplers[(uint32_t)sampler];
	}
	HBBR_API HBBR_INLINE static uint64_t GetTextureStreamingSize() {
		return _textureStreamingSize;
	}

	HBBR_API void Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);

	HBBR_API void TransitionImmediate(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);

	HBBR_API bool CopyBufferToTexture(VkCommandBuffer cmdbuf);

	HBBR_API void CopyBufferToTextureImmediate();

	void DestoryUploadBuffer();

	HBBR_API bool CopyTextureToBuffer(VkCommandBuffer cmdbuf, Buffer* buffer, VkDeviceSize offset = 0);

	HBBR_API void CopyTextureToBufferImmediate(Buffer* buffer, VkDeviceSize offset = 0);

	HBBR_API void Resize(uint32_t width, uint32_t height);

	HBBR_API HBBR_INLINE bool IsValid()const {
		return _bUploadToGPU > VulkanManager::GetManager()->GetSwapchainBufferCount();
	}

	HBBR_API static std::shared_ptr<Texture2D> CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName = "Texture2D", uint32_t miplevel = 1, uint32_t layerCount = 1, VkMemoryPropertyFlags memoryPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	HBBR_API static std::shared_ptr<Texture2D> LoadAsset(HGUID guid , VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	HBBR_API virtual void SaveAsset(HString path)override;

	void UploadToGPU();

	static void GlobalInitialize();

	static void GlobalUpdate();

	static void GlobalRelease();

	HBBR_API static void AddSystemTexture(HString tag, std::weak_ptr<Texture2D> tex);

	//获取渲染系统纹理,如果查找失败则返回第一张
	HBBR_API static Texture2D* GetSystemTexture(HString tag);

	HString _textureName;

	//Image data, Read only
	ImageData _imageData;

protected:

	uint32_t _bUploadToGPU = 0;

	//Vulkan object
	VkImage _image;
	VkImageView _imageView;
	VkDeviceMemory _imageViewMemory;
	VkFormat _format;
	VkImageUsageFlags _usageFlags;
	VkImageAspectFlags _imageAspectFlags;
	uint32_t _mipCount = 1;
	VkExtent3D _imageSize;
	uint64_t _textureMemorySize =0;
	uint8_t _mipBias = 0;
	VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	//Upload object
	VkBuffer _uploadBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _uploadBufferMemory = VK_NULL_HANDLE;
	static std::vector<Texture2D*> _upload_textures;

	//Global variable
	static std::unordered_map<HString, std::weak_ptr<Texture2D>> _system_textures;
	//<sampler>
	static std::vector<VkSampler> _samplers;

	//Texture2D streaming
	static uint64_t _textureStreamingSize;
	static uint64_t _maxTextureStreamingSize;
};
