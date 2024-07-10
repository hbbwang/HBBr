﻿#include "Buffer.h"
#include "VulkanManager.h"

Buffer::Buffer(VkBufferUsageFlags usageFlags, uint64_t bufferSize)
{
	CreateBuffer(usageFlags, bufferSize);
}

Buffer::~Buffer()
{
	VulkanManager::GetManager()->FreeBufferMemory(_bufferMemory);
	VulkanManager::GetManager()->DestroyBuffer(_buffer);
}

void Buffer::Resize(uint64_t newSize)
{
	UnMapMemory();
	_bufferCapacity = newSize;
	//保存旧缓存,保证安全释放,我们必须保证Buffer不再被使用的时候再释放
	//从GetBuffer里进行帧计算
	BufferWaitToRelease oldBuffer;
	oldBuffer.old_buffer = std::move(_buffer);
	oldBuffer.old_bufferMemory = std::move(_bufferMemory);
	_oldBuffer.push_back(oldBuffer);
	//创建新Buffer
	VulkanManager::GetManager()->CreateBuffer(_bufferUsage, newSize, _buffer);
	VulkanManager::GetManager()->AllocateBufferMemory(_buffer, _bufferMemory);
	MapMemory();
}

void Buffer::AddSize(uint64_t newSize)
{
	_bufferCapacity += newSize;
	Resize(_bufferCapacity);
}

void Buffer::CreateBuffer(VkBufferUsageFlags usageFlags, uint64_t bufferSize)
{
	if (_buffer != VK_NULL_HANDLE)
		return;
	_bufferUsage = usageFlags;
	_bufferCapacity = bufferSize;
	VulkanManager::GetManager()->CreateBuffer(usageFlags, bufferSize, _buffer);
	VulkanManager::GetManager()->AllocateBufferMemory(_buffer, _bufferMemory);
	MapMemory();
}

void Buffer::MapMemory()
{
	if (_bufferIsMapping)
		return;
	_bufferIsMapping = true;
	auto result = vkMapMemory(VulkanManager::GetManager()->GetDevice(), _bufferMemory, 0, VK_WHOLE_SIZE, 0, &_bufferMapping);
	if (result != VK_SUCCESS)
	{
	}
}

void Buffer::UnMapMemory()
{
	if (!_bufferIsMapping)
		return;
	_bufferIsMapping = false;
	vkUnmapMemory(VulkanManager::GetManager()->GetDevice(), _bufferMemory);
}

VkBuffer Buffer::GetBuffer()
{
	//销毁计数旧Buffer
	const size_t oldBufferCount = _oldBuffer.size();
	for (size_t i = 0; i < oldBufferCount; i++)
	{
		if (_oldBuffer[i].safeDestoryOldBuffer > (int)VulkanManager::GetManager()->GetSwapchainBufferCount())
		{
			VulkanManager::GetManager()->FreeBufferMemory(_oldBuffer[i].old_bufferMemory);
			VulkanManager::GetManager()->DestroyBuffer(_oldBuffer[i].old_buffer);
			_oldBuffer.erase(_oldBuffer.begin() + i);
			break;
		}
		_oldBuffer[i].safeDestoryOldBuffer++;
	}
	return _buffer;
}

VkDeviceMemory Buffer::GetMemory()
{
	return _bufferMemory;
}
