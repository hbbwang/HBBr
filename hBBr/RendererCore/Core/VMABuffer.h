#pragma once
#include "Common.h"
#include "VulkanManager.h"

//每次顶点Buffer最大增加的大小 :
//(UINT32_MAX/1024)约等于2M
#define VMABufferSizeRange (UINT32_MAX/1024)

//Uniform Buffer每次增长大小
//Uniform Buffer好像是有大小限制的:VulkanManager->GetMaxUniformBufferSize()
#define VMAUniformBufferSizeRange (512)

class VMABuffer
{
public:
	VMABuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY, bool bAlwayMapping = false, bool bFocusCreateDedicatedMemory = false, HString debugName = "VMABuffer");
	~VMABuffer();

	//最后的VkCommandBuffer是给VMA_MEMORY_USAGE_GPU_ONLY准备的
	//如果是nullptr，会自动创建一个，并且强行等待执行完成，效率较低。  
	void Mapping(void* data, VkDeviceSize offset, VkDeviceSize dataSize);  

	//Buffer大小重置,如果是从大的重置成小的,原来的数据将会丢失
	bool Resize(VkDeviceSize newSize, VkCommandBuffer cmdBuf = VK_NULL_HANDLE);

	void* BeginMapping();

	void EndMapping();

	static inline VkDeviceSize GetMaxAlignmentSize(VkDeviceSize targetSize, VkDeviceSize AlignSize) {
		return (targetSize + AlignSize - 1) & ~(AlignSize - 1);
	}

	inline VkBuffer GetBuffer()const {
		return _buffer;
	}

	inline VkDeviceSize GetBufferSize()const {
		return _allocationInfo.size;
	}

private:

	void AlignmentSize(VkDeviceSize& sizeInout);

	VkBuffer _buffer; 

	VmaAllocation _allocation;

	VmaAllocationInfo _allocationInfo;

	bool _bAlwayMapping;

	VkBufferUsageFlags _bufferUsage;

	VmaMemoryUsage _memoryUsage;

	HString _debugName;

	bool _bFocusCreateDedicatedMemory;

	bool _gpuOnlyDataCopy;

	VkMemoryPropertyFlags _memFlags;
};
