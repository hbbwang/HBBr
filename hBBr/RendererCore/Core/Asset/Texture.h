﻿#pragma once
//基层HObject,管理对象
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
//Vulkan api
#include "VulkanManager.h"

#ifdef IS_EDITOR
#include "nvtt/include/nvtt/nvtt.h"
#pragma comment(lib,"nvtt/lib/x64-v142/nvtt30106.lib")
#endif

struct FontTextureInfo
{
	//Font data
	float posX;
	float posY;
	float sizeX;
	float sizeY;
	float sizeOffsetX;
};

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
};

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
	inline std::shared_ptr<Texture> GetTexture(SceneTextureDesc desc)
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

class Texture : public AssetObject
{
	friend class VulkanManager;
	friend class ContentManager;
public:
	Texture() {}
	~Texture();
	HBBR_INLINE VkImage GetTexture()const {
		return _image;
	}
	HBBR_INLINE VkImageView GetTextureView()const {
		return _imageView;
	}
	HBBR_INLINE VkFormat GetFormat()const {
		return _format;
	}
	HBBR_INLINE VkImageLayout GetLayout ()const {
		return _imageLayout;
	}
	HBBR_INLINE VkImageUsageFlags GetUsageFlags()const {
		return _usageFlags;
	}
	HBBR_INLINE uint32_t GetMipCount()const {
		return _mipCount;
	}
	HBBR_INLINE VkImageAspectFlags GetAspectFlags()const {
		return _imageAspectFlags;
	}

	HBBR_INLINE VkExtent2D GetImageSize()const {
		return _imageSize;
	}

	HBBR_INLINE static std::vector<Texture*>& GetUploadTextures(){
		return _upload_textures;
	}

	HBBR_INLINE static Texture* GetFontTexture() {
		return _fontTexture.get();
	}

	HBBR_INLINE static VkSampler GetSampler(TextureSampler sampler , int mipBias = -1) {
		auto samplers = _samplers[sampler];
		if (mipBias <0 || (int)samplers.size() <= mipBias)
		{
			mipBias = 0;
		}
		return samplers[mipBias];
	}

	HBBR_INLINE static FontTextureInfo* GetFontInfo(wchar_t c) {
		auto it = _fontTextureInfos.find(c);
		if (it != _fontTextureInfos.end())
		{
			return &it->second;
		}
		else
		{
			return &_fontTextureInfos[32];
		}
	}

	HBBR_INLINE static uint64_t GetTextureStreamingSize() {
		return _textureStreamingSize;
	}

	HBBR_API void Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);

	HBBR_API void TransitionImmediate(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1);

	HBBR_API bool CopyBufferToTexture(VkCommandBuffer cmdbuf);

	HBBR_API void CopyBufferToTextureImmediate();

	HBBR_API void Resize(uint32_t width, uint32_t height);

	HBBR_INLINE bool IsValid()const {
		return _bUploadToGPU;
	}

	HBBR_API static std::shared_ptr<Texture> CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName = "Texture", uint32_t miplevel = 1, uint32_t layerCount = 1);

	HBBR_API static std::weak_ptr<Texture> LoadAsset(HGUID guid , VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	static void GlobalInitialize();

	static void GlobalUpdate();

	static void GlobalRelease();

	HBBR_API static void AddSystemTexture(HString tag, Texture* tex);

	//获取渲染系统纹理,如果查找失败则返回第一张
	HBBR_API static Texture* GetSystemTexture(HString tag);

	//通过ttf生成dds纹理
	static void CreateFontTexture(HString ttfFontPath, HString outTexturePath,bool bOverwrite = true,uint32_t fontSize = 48 , uint32_t maxTextureSize = 256);

	HString _textureName;

#ifdef IS_EDITOR

#pragma region NVTT
	HBBR_API static void CompressionImage2D(const char* imagePath, const char* outputDDS, bool bGenerateMips, nvtt::Format format, bool bGenerateNormalMap, bool bAutoFormat = false);
	//HBBR_API static void CompressionImageCube(const char* imagePath, const char* outputDDS, bool bGenerateMips);
	HBBR_API static void DecompressionImage2D(const char* ddsPath, const char* outputPath, nvtt::Surface* outData = NULL, int32_t newWidth = -1, int32_t newHeight = -1, int32_t newDepth = -1);
	HBBR_API static void DecompressionImageCube(const char* ddsPath, const char* outputPath, nvtt::Surface* outData = NULL, int32_t newWidth = -1, int32_t newHeight = -1, int32_t newDepth = -1);
	HBBR_API static void OutputImage(const char* outputPath, int w, int h, nvtt::Format format , void* outData);
	HBBR_API static void GetImageDataFromCompressionData(const char* ddsPath, nvtt::Surface* outData);
#pragma endregion NVTT

#endif

private:

	bool _bUploadToGPU = false;

	//Vulkan object
	VkImage _image;
	VkImageView _imageView;
	VkDeviceMemory _imageViewMemory;
	VkFormat _format;
	VkImageUsageFlags _usageFlags;
	VkImageAspectFlags _imageAspectFlags;
	uint32_t _mipCount = 1;
	VkExtent2D _imageSize;
	uint64_t _textureMemorySize =0;
	uint8_t _mipBias = 0;
	VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	//Image data
	ImageData* _imageData = NULL;

	//Upload object
	VkBuffer _uploadBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _uploadBufferMemory = VK_NULL_HANDLE;
	static std::vector<Texture*> _upload_textures;

	//Global variable
	static std::unordered_map<HString, Texture*> _system_textures;
	//<mipLod,sampler>
	static std::unordered_map<TextureSampler, std::vector<VkSampler>> _samplers;
	
	// vector<RGBA channel<wchar_t , FontTextureInfo>>
	static std::unordered_map<wchar_t, FontTextureInfo> _fontTextureInfos;
	static std::shared_ptr<Texture> _fontTexture;

	//Texture streaming
	static uint64_t _textureStreamingSize;
	static uint64_t _maxTextureStreamingSize;
};