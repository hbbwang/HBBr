#pragma once
//基层HObject,管理对象
#include "Common.h"
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>
#include "./Resource/HGuid.h"
#include "HString.h"
#include "ImageTool.h"
//Vulkan api
#include "VulkanManager.h"

#ifdef IS_EDITOR
#include "nvtt/include/nvtt/nvtt.h"
#pragma comment(lib,"nvtt/lib/x64-v142/nvtt30106.lib")
#endif

enum TextureSampler
{
	TextureSampler_Linear_Wrap = 0,
	TextureSampler_Linear_Mirror = 1,
	TextureSampler_Linear_Clamp = 2,
	TextureSampler_Nearest_Wrap = 3,
	TextureSampler_Nearest_Mirror = 4,
	TextureSampler_Nearest_Clamp = 5,
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

class Texture
{
	friend class VulkanManager;
public:
	Texture() {}
	Texture(bool bNoMemory) {_bNoMemory = bNoMemory;}
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

	HBBR_INLINE VkSampler GetSampler()const {
		return _sampler;
	}

	HBBR_INLINE VkExtent2D GetImageSize()const {
		return _imageSize;
	}

	HBBR_INLINE void SetSampler(TextureSampler sampler) {
		_sampler = _samplers[sampler];
	}

	HBBR_INLINE static std::vector<Texture*>& GetUploadTextures(){
		return _upload_textures;
	}

	void Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);

	void TransitionImmediate(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin = 0, uint32_t mipLevelCount = 1);

	bool CopyBufferToTexture(VkCommandBuffer cmdbuf);

	void CopyBufferToTextureImmediate();

	void Resize(uint32_t width, uint32_t height);

	HBBR_INLINE bool IsValid()const {
		return _bUploadToGPU;
	}

	static std::shared_ptr<Texture> CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName = "Texture", bool noMemory = false);

	static Texture* ImportTextureAsset(HGUID guid , VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	static void GlobalInitialize();

	static void GlobalUpdate();

	static void GlobalRelease();

	static void AddSystemTexture(HString tag, Texture* tex);

	static Texture* GetSystemTexture(HString tag);

	//通过ttf生成dds纹理
	static void CreateFontTexture(HString ttfFontPath, HString outTexturePath,bool bOverwrite = false);

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
	VkSampler _sampler;
	//static std::unordered_map<HGUID, Texture> _all_textures;
	ImageData* _imageData = NULL;

	//Upload object
	VkBuffer _uploadBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _uploadBufferMemory = VK_NULL_HANDLE;
	static std::vector<Texture*> _upload_textures;

	//Global variable
	static std::unordered_map<HString, Texture*> _system_textures;
	static std::unordered_map < TextureSampler, VkSampler > _samplers;
};
