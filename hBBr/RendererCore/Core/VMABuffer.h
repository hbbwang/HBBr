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
	void Mapping(void* data, VkDeviceSize offset, VkDeviceSize dataSize, VkCommandBuffer cmdBuf = VK_NULL_HANDLE);

	bool Resize(VkDeviceSize newSize);

	//扩容
	bool ResizeBigger(VkDeviceSize newSize);

	static inline VkDeviceSize GetMaxAlignmentSize(VkDeviceSize targetSize, VkDeviceSize AlignSize) {
		return (targetSize + AlignSize - 1) & ~(AlignSize - 1);
	}

	inline VkBuffer GetBuffer()const {
		return _buffer;
	}
	inline VkDeviceSize GetBufferSize()const {
		return _lastSize;
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

	HString _debugName;

	bool _bFocusCreateDedicatedMemory;
};
