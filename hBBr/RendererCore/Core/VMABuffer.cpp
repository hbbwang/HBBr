#include "VMABuffer.h"
#include "ConsoleDebug.h"
#include "RendererConfig.h"
#include "VulkanObjectManager.h"

VMABuffer::VMABuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, bool bAlwayMapping, bool bFocusCreateDedicatedMemory):
	_lastSize(bufferSize),
	_bufferUsage(bufferUsage),
	_memoryUsage(memoryUsage),
	_bAlwayMapping(bAlwayMapping),
	_bFocusCreateDedicatedMemory(bFocusCreateDedicatedMemory)
{
	if (bufferSize <= 0)//不能创建一个大小为0的Buffer
	{
		bufferSize = _lastSize = 1;
	}
	const auto& manager = VulkanManager::GetManager();
	manager->VMACraeteBufferAndAllocateMemory(_lastSize, _bufferUsage, _buffer, _allocation, &_allocationInfo, _memoryUsage, _bAlwayMapping, _bFocusCreateDedicatedMemory);
#if IS_EDITOR
	ConsoleDebug::printf_endl_succeed(GetInternationalizationText("Renderer", "CreateBuffer"), _allocationInfo.size, (double)_allocationInfo.size / (double)1024.0 / (double)1024.0);
#endif
}

VMABuffer::~VMABuffer()
{
	const auto& manager = VulkanManager::GetManager();
	manager->VMADestroyBufferAndFreeMemory(_buffer, _allocation);
}

void VMABuffer::Mapping(void* data, VkDeviceSize offset, VkDeviceSize dataSize)
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

			void* bufBegin = (uint8_t*)(mappedData)+offset;
			memcpy(bufBegin, data, dataSize);

			vmaUnmapMemory(manager->GetVMA(), _allocation);

			//if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			//	vmaFlushAllocation(manager->GetVMA(), _allocation, );
		}
	}
	else
	{
		void* bufBegin = (uint8_t*)(_allocationInfo.pMappedData) + offset;
		memcpy(bufBegin, data, dataSize);
	}
}

bool VMABuffer::Resize(VkDeviceSize newSize)
{
	if (newSize == _lastSize || newSize <= 0)
	{
		return false;
	}

	const auto& manager = VulkanManager::GetManager();

	//创建新Buffer
	VkBuffer newBuffer;
	VmaAllocation newAllocation;
	VmaAllocationInfo newAllocationInfo;
	manager->VMACraeteBufferAndAllocateMemory(newSize, _bufferUsage, newBuffer, newAllocation, &newAllocationInfo, _memoryUsage, _bAlwayMapping, _bFocusCreateDedicatedMemory);

	//把旧Buffer数据复制到新的上面
	{
		VkMemoryPropertyFlags memFlags;
		vmaGetAllocationMemoryProperties(manager->GetVMA(), _allocation, &memFlags);
		if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
		{
			void* oldMappedData;
			void* newMappedData;
			vmaMapMemory(manager->GetVMA(), _allocation,	&oldMappedData);
			vmaMapMemory(manager->GetVMA(), newAllocation,	&newMappedData);
			memcpy(newMappedData, oldMappedData, _lastSize);
			vmaUnmapMemory(manager->GetVMA(), _allocation);
			vmaUnmapMemory(manager->GetVMA(), newAllocation);
		}
		else
		{
			memcpy(newAllocationInfo.pMappedData, _allocationInfo.pMappedData, _lastSize);
		}
	}

	//释放旧Buffer
	VulkanObjectManager::Get()->SafeReleaseVMABuffer(_buffer, _allocation);

	_lastSize = newSize;
	_buffer = newBuffer;
	_allocation = newAllocation;
	_allocationInfo = newAllocationInfo;

#if IS_EDITOR
	ConsoleDebug::printf_endl_succeed(GetInternationalizationText("Renderer", "ResizeBuffer"), _allocationInfo.size, (double)_allocationInfo.size / (double)1024.0 / (double)1024.0);
#endif

	return true;
}
