#pragma once
#include "Common.h"
#include "VulkanManager.h"

//每次顶点Buffer最大增加的大小 :
//(UINT32_MAX/512)约等于8M
#define VMABufferSizeRange (UINT32_MAX/512)

//Uniform Buffer每次增长大小
//Uniform Buffer好像是有大小限制的:VulkanManager->GetMaxUniformBufferSize()
#define VMAUniformBufferSizeRange (512)

class VMABuffer
{
public:
	VMABuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY, bool bAlwayMapping = false, bool bFocusCreateDedicatedMemory = false);
	~VMABuffer();

	void Mapping(void* data, VkDeviceSize offset, VkDeviceSize dataSize);

	bool Resize(VkDeviceSize newSize);

	inline VkBuffer GetBuffer()const {
		return _buffer;
	}

private:

	VkBuffer _buffer; 

	VmaAllocation _allocation;

	VmaAllocationInfo _allocationInfo;

	//上一次buffer所需的大小，这并不是Buffer的内存大小，因为它还未进行对齐，只是用来进行Resize判断的。
	VkDeviceSize _lastSize;

	bool _bAlwayMapping;

	VkBufferUsageFlags _bufferUsage;

	VmaMemoryUsage _memoryUsage;

	bool _bFocusCreateDedicatedMemory;
};
