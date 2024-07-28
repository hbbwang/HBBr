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

	void Mapping(void* data, VkDeviceSize dataSize);

private:
	VkBuffer _buffer; 
	VmaAllocation _allocation;
	VmaAllocationInfo _allocationInfo;
	bool _bAlwayMapping;
};
