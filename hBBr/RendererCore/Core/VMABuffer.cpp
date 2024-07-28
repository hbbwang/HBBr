#include "VMABuffer.h"

VMABuffer::VMABuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, bool bAlwayMapping, bool bFocusCreateDedicatedMemory):
	_bAlwayMapping(bAlwayMapping)
{
	const auto& manager = VulkanManager::GetManager();
	manager->VMACraeteBufferAndAllocateMemory(bufferSize, bufferUsage, _buffer, _allocation, memoryUsage, bAlwayMapping, bFocusCreateDedicatedMemory);
}

VMABuffer::~VMABuffer()
{
	const auto& manager = VulkanManager::GetManager();
	manager->VMADestroyBufferAndFreeMemory(_buffer, _allocation);
}

void VMABuffer::Mapping(void* data, VkDeviceSize dataSize)
{
	if (!_bAlwayMapping)
	{
		const auto& manager = VulkanManager::GetManager();
		VkMemoryPropertyFlags memFlags;
		vmaGetAllocationMemoryProperties(manager->GetVMA(), _allocation, &memFlags);
		if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
		{
			//if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			//	vmaInvalidateAllocation(manager->GetVMA(), _allocation,);

			void* mappedData;
			vmaMapMemory(manager->GetVMA(), _allocation, &mappedData);
			memcpy(mappedData, data, dataSize);
			vmaUnmapMemory(manager->GetVMA(), _allocation);

			//if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			//	vmaFlushAllocation(manager->GetVMA(), _allocation, );
		}
	}
	else
	{
		memcpy(_allocationInfo.pMappedData, data, dataSize);
	}
}
