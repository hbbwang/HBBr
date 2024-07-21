#include "Buffer.h"
#include "VulkanManager.h"
#include "VulkanObjectManager.h"
#include "ConsoleDebug.h"
#include "RendererConfig.h"
Buffer::Buffer(VkBufferUsageFlags usageFlags, uint64_t bufferSize)
{
	CreateBuffer(usageFlags, bufferSize);
}

Buffer::~Buffer()
{
	//VulkanManager::GetManager()->FreeBufferMemory(_bufferMemory);
	//VulkanManager::GetManager()->DestroyBuffer(_buffer);
	//for (size_t i = 0; i < _oldBuffer.size(); i++)
	//{
	//	VulkanManager::GetManager()->FreeBufferMemory(_oldBuffer[i].old_bufferMemory);
	//	VulkanManager::GetManager()->DestroyBuffer(_oldBuffer[i].old_buffer);
	//}
	_buffer.reset();
	_bufferMemory.reset();
	//_oldBuffer.clear();
}

bool Buffer::Resize(uint64_t newSize, bool bForceResize)
{
	if (_bufferCapacity >= newSize && !bForceResize)
	{
		return false;
	}
	UnMapMemory();
	_bufferCapacity = newSize;
	//保存旧缓存,保证安全释放,我们必须保证Buffer不再被使用的时候再释放
	//从GetBuffer里进行帧计算
 
	//BufferWaitToRelease oldBuffer;
	//oldBuffer.old_buffer = std::move(_buffer);
	//oldBuffer.old_bufferMemory = std::move(_bufferMemory);
	//if (_oldBuffer.capacity() == 0)
	//{
	//	_oldBuffer.reserve(4);
	//}
	//_oldBuffer.push_back(oldBuffer);

	//创建新Buffer
	_buffer = VulkanObjectManager::Get()->CreateVkBuffer(_bufferUsage, _bufferCapacity);
	_bufferMemory = VulkanObjectManager::Get()->AllocateVkDeviceMemory(*_buffer);

#if IS_EDITOR
	ConsoleDebug::printf_endl_succeed(GetInternationalizationText("Renderer", "ResizeBuffer"), _bufferCapacity , (double)_bufferCapacity/ (double)1024.0/ (double)1024.0);
#endif

	MapMemory();
	return true;
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
	_buffer = VulkanObjectManager::Get()->CreateVkBuffer(usageFlags, bufferSize);
	_bufferMemory = VulkanObjectManager::Get()->AllocateVkDeviceMemory(*_buffer);

#if IS_EDITOR
	ConsoleDebug::printf_endl_succeed(GetInternationalizationText("Renderer", "CreateBuffer"), _bufferCapacity, (double)_bufferCapacity / (double)1024.0 / (double)1024.0);
#endif

	MapMemory();
}

void Buffer::MapMemory()
{
	if (_bufferIsMapping)
		return;
	_bufferIsMapping = true;
	auto result = vkMapMemory(VulkanManager::GetManager()->GetDevice(), *_bufferMemory, 0, VK_WHOLE_SIZE, 0, &_bufferMapping);
	if (result != VK_SUCCESS)
	{
	}
}

void Buffer::UnMapMemory()
{
	if (!_bufferIsMapping)
		return;
	_bufferIsMapping = false;
	vkUnmapMemory(VulkanManager::GetManager()->GetDevice(), *_bufferMemory);
}

VkBuffer Buffer::GetBuffer()
{
	//销毁计数旧Buffer

	//const size_t oldBufferCount = _oldBuffer.size();
	//for (size_t i = 0; i < oldBufferCount; i++)
	//{
	//	if (_oldBuffer[i].safeDestoryOldBuffer > (int)VulkanManager::GetManager()->GetSwapchainBufferCount())
	//	{
	//		VulkanManager::GetManager()->FreeBufferMemory(_oldBuffer[i].old_bufferMemory);
	//		VulkanManager::GetManager()->DestroyBuffer(_oldBuffer[i].old_buffer);
	//		_oldBuffer.erase(_oldBuffer.begin() + i);
	//		break;
	//	}
	//	_oldBuffer[i].safeDestoryOldBuffer++;
	//}

	return *_buffer;
}

VkDeviceMemory Buffer::GetMemory()
{
	return *_bufferMemory;
}
