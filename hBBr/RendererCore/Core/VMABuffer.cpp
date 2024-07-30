#include "VMABuffer.h"
#include "ConsoleDebug.h"
#include "RendererConfig.h"
#include "VulkanObjectManager.h"

VMABuffer::VMABuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, bool bAlwayMapping, bool bFocusCreateDedicatedMemory, HString debugName):
	_lastSize(bufferSize),
	_bufferUsage(bufferUsage),
	_memoryUsage(memoryUsage),
	_bAlwayMapping(bAlwayMapping),
	_bFocusCreateDedicatedMemory(bFocusCreateDedicatedMemory),
	_debugName(debugName),
	_gpuOnlyDataCopy(false)
{
	if (bufferSize <= 0)//不能创建一个大小为0的Buffer
	{
		bufferSize = _lastSize = 1;
	}
	const auto& manager = VulkanManager::GetManager();

	if (_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)//仅GPU可见，复制数据需要媒介
	{
		_bufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT; //为了支持Copy和Resize
	}

	manager->VMACraeteBufferAndAllocateMemory(_lastSize, _bufferUsage, _buffer, _allocation, &_allocationInfo, _memoryUsage, _bAlwayMapping, _bFocusCreateDedicatedMemory);

	vmaGetAllocationMemoryProperties(manager->GetVMA(), _allocation, &_memFlags);

	#if IS_EDITOR
		ConsoleDebug::printf_endl_succeed(GetInternationalizationText("Renderer", "CreateBuffer"), _debugName.c_str(), _allocationInfo.size, (double)_allocationInfo.size / (double)1024.0 / (double)1024.0);
	#endif
}

VMABuffer::~VMABuffer()
{
	const auto& manager = VulkanManager::GetManager();
	manager->VMADestroyBufferAndFreeMemory(_buffer, _allocation, _debugName, _allocationInfo.size);
}

void VMABuffer::Mapping(void* data, VkDeviceSize offset, VkDeviceSize dataSize, VkCommandBuffer cmdBuf)
{
	if (_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)//仅GPU可见的内存，是不能从CPU复制数据的，必须通过"媒介"
	{
		const auto& manager = VulkanManager::GetManager();
		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		manager->VMACraeteBufferAndAllocateMemory(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, stagingAllocation, nullptr, VMA_MEMORY_USAGE_CPU_TO_GPU, false, true);
		// 将数据写入到 "staging" 缓冲区
		void* mappedData;
		vmaMapMemory(manager->GetVMA(), stagingAllocation, &mappedData);
		void* bufBegin = (uint8_t*)(mappedData)+offset;
		memcpy(bufBegin, data, dataSize);
		vmaUnmapMemory(manager->GetVMA(), stagingAllocation);
		//将数据从"staging" 缓冲区复制到目标缓冲区
		manager->CmdBufferCopyToBuffer(cmdBuf, stagingBuffer, _buffer, dataSize);
		//销毁"staging" 缓冲区
		manager->VMADestroyBufferAndFreeMemory(stagingBuffer, stagingAllocation);
		if (_gpuOnlyDataCopy)
		{
			ConsoleDebug::printf_endl_warning(GetInternationalizationText("Renderer", "BufferCopyGPUOnly"), _debugName.c_str());
		}
		_gpuOnlyDataCopy = true;
	}
	else
	{
		if (!_bAlwayMapping)
		{
			const auto& manager = VulkanManager::GetManager();
			if ((_memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
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
}

bool VMABuffer::Resize(VkDeviceSize newSize, VkCommandBuffer cmdBuf)
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
	vmaGetAllocationMemoryProperties(manager->GetVMA(), newAllocation, &_memFlags);
	//把旧Buffer数据复制到新的上面
	{
		if (_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)
		{
			//老Buffer复制到新Buffer
			manager->CmdBufferCopyToBuffer(cmdBuf, _buffer, newBuffer, std::min(_lastSize, newSize));
		}
		else
		{
			if ((_memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
			{
				void* oldMappedData;
				void* newMappedData;
				vmaMapMemory(manager->GetVMA(), _allocation, &oldMappedData);
				vmaMapMemory(manager->GetVMA(), newAllocation, &newMappedData);
				memcpy(newMappedData, oldMappedData, std::min(_lastSize, newSize));
				vmaUnmapMemory(manager->GetVMA(), _allocation);
				vmaUnmapMemory(manager->GetVMA(), newAllocation);
			}
			else
			{
				memcpy(newAllocationInfo.pMappedData, _allocationInfo.pMappedData, std::min(_lastSize, newSize));
			}
		}		
	}

	//释放旧Buffer
	VulkanObjectManager::Get()->SafeReleaseVMABuffer(_buffer, _allocation);

	_lastSize = newSize;
	_buffer = newBuffer;
	_allocation = newAllocation;
	_allocationInfo = newAllocationInfo;

#if IS_EDITOR
	ConsoleDebug::printf_endl_succeed(GetInternationalizationText("Renderer", "ResizeBuffer"), _debugName.c_str(), _allocationInfo.size, (double)_allocationInfo.size / (double)1024.0 / (double)1024.0);
#endif

	return true;
}

bool VMABuffer::ResizeBigger(VkDeviceSize newSize, VkCommandBuffer cmdBuf)
{
	if (newSize > _lastSize)
	{
		return Resize(newSize, cmdBuf);
	}	
	return false;
}

void* VMABuffer::BeginMapping()
{
	const auto& manager = VulkanManager::GetManager();
	if (_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)
		return nullptr;
	if ((_memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
	{
		void* mappedData;
		vmaMapMemory(manager->GetVMA(), _allocation, &mappedData);
		return mappedData;
	}
	return _allocationInfo.pMappedData;
}

void VMABuffer::EndMapping()
{
	const auto& manager = VulkanManager::GetManager();
	if (_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)
		return;
	if ((_memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
		vmaUnmapMemory(manager->GetVMA(), _allocation);
}
