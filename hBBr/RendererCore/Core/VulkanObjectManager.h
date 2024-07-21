#pragma once
#include "Common.h"
#include "VulkanManager.h"
#include "HTime.h"
#include <vector>
#include <memory>
#include <thread>

struct VkBufferObject
{
	std::shared_ptr<VkBuffer> buffer;
	uint8_t bImmediate = true;
	uint8_t frameCount = 0;
};

struct VkDeviceMemoryObject
{
	std::shared_ptr<VkDeviceMemory> buffer;
	uint8_t bImmediate = true;
	uint8_t frameCount = 0;
};

//在这里创建的Vulkan对象，拥有最简单的垃圾回收机制
class VulkanObjectManager
{
	friend class VulkanApp;
public:
	HBBR_API HBBR_INLINE static VulkanObjectManager* Get() {
		if (!_vulkanObjectManager)
		{
			_vulkanObjectManager.reset(new VulkanObjectManager);
		}
		return _vulkanObjectManager.get();
	}

	HBBR_API HBBR_INLINE double GetGCTime()
	{
		return _gcCurrentSecond;
	}

	//设置GC的时间间隔，建议不要少于10秒
	HBBR_API HBBR_INLINE void SetGCInterval(uint32_t interval)
	{
		_gcMaxSecond = interval;
	}

	//bImmediate = true的时候,代表如果没有引用了，只需要等待3帧即可销毁，不然就还需要等待GC倒计时
	std::shared_ptr<VkBuffer> CreateVkBuffer(VkBufferUsageFlags usage, VkDeviceSize bufferSize, bool bImmediate = true);

	std::shared_ptr<VkDeviceMemory> AllocateVkDeviceMemory(VkBuffer buffer, VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bool bImmediate = true);

protected:
	void Update();
	void Release();
private:
	static std::unique_ptr<VulkanObjectManager>		_vulkanObjectManager;
	//Objects
	
	std::vector<VkBufferObject*>_bufferObjects;
	std::vector<VkDeviceMemoryObject*>_deviceMemoryObjects;
	
	//
	double _gcCurrentSecond = 0;
	double _gcMaxSecond = 40;
	HTime _gcTime;
	uint32_t _numRequestObjects = 0;
};