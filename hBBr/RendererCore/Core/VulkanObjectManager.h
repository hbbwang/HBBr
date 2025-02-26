﻿#pragma once
#include "Common.h"
#include "VulkanManager.h"
#include "HTime.h"
#include <vector>
#include <list>
#include <memory>
#include <thread>
#include <typeinfo>

struct VkAssetObject
{
	std::weak_ptr<class AssetObject> asset;
	uint8_t frameCount = 0;
	bool bImmediate = false;
};

struct VkBufferObject
{
	VkBuffer buffer;
	VkDeviceMemory memory;
	uint8_t frameCount = 0;
};

struct VkTextureRefreshObject
{
	std::weak_ptr<class Texture2D> texture;
	uint8_t frameCount = 0;
	uint8_t bFocusWaitIdle = 0;
};

struct VkTextureDestroyObject
{
	VkDeviceMemory memory = nullptr;
	VkImage image = nullptr;
	VkImageView imageView = nullptr;
	uint8_t frameCount = 0;
#if _DEBUG
	std::string tag = "";
#endif
};

struct VMABufferObject
{
	VkBuffer buffer;
	VmaAllocation allocation;
	std::string debugName = "VMABuffer";
	VkDeviceSize debugSize = 0;
	uint8_t frameCount = 0;
};

//在这里创建的Vulkan对象，拥有最简单的垃圾回收机制
class VulkanObjectManager
{
	friend class VulkanApp;
	friend class VkPtrBase;

public:

	HBBR_API inline static VulkanObjectManager* Get() {
		if (!_ptr)
			_ptr.reset(new VulkanObjectManager());
		return _ptr.get();
	}

	HBBR_API HBBR_INLINE double GetGCTime()const
	{
		return _gcCurrentSecond;
	}

	HBBR_API HBBR_INLINE double GetMaxGCTime()const
	{
		return _gcMaxSecond;
	}

	//设置GC的时间间隔，建议不要少于10秒
	HBBR_API HBBR_INLINE void SetGCInterval(uint32_t interval)
	{
		_gcMaxSecond = interval;
	}

	void SafeReleaseVkBuffer(VkBuffer buffer, VkDeviceMemory memory);

	void SafeReleaseVMABuffer(VkBuffer buffer, VmaAllocation allocation, std::string debugName = "VMABuffer", VkDeviceSize debugSize = 0);

	void AssetLinkGC(std::weak_ptr<class AssetObject> asset,bool bImmediate = false);

	void RefreshTexture(std::weak_ptr<class Texture2D> texture, uint8_t bFocusWaitIdle = 0);

	void SafeReleaseTexture(VkImage image, VkImageView imageView, VkDeviceMemory memory, std::string tag = "");

protected:

	void Update();

	void Release();

private:
	VulkanObjectManager();

	static std::unique_ptr<VulkanObjectManager> _ptr;

	std::vector<VkBufferObject*>_vkBufferObjects;

	std::vector<VMABufferObject*>_vmaBufferObjects;

	std::map<class Texture2D*, VkTextureRefreshObject> _textureRefresh;

	std::list<VkTextureDestroyObject> _textureDestroy;

	std::list<VkAssetObject> _vulkanObjects;

	std::list<VkAssetObject> _vulkanObjectsRelease;

	uint32_t _assetCheckCount;
	uint32_t _maxAssetCheckCount;
	bool _bStartCheckAsset;

	double _gcCurrentSecond;
	double _gcMaxSecond;
	HTime _gcTime;
	uint32_t _numRequestObjects;
};