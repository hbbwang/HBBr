#include "Buffer.h"
#include "VulkanManager.h"
#include "VulkanObjectManager.h"
#include "ConsoleDebug.h"
#include "RendererConfig.h"
Buffer::Buffer(VkBufferUsageFlags usageFlags, uint64_t bufferSize):
	_buffer(VK_NULL_HANDLE),
	_bufferMemory(VK_NULL_HANDLE),
	_bufferCapacity(bufferSize),
	_bufferUsage(usageFlags),
	_bufferMapping(nullptr),
	_bufferIsMapping(false)
{
	CreateBuffer(usageFlags, bufferSize);
}

Buffer::~Buffer()
{
	VulkanManager::GetManager()->FreeBufferMemory(_bufferMemory);
	VulkanManager::GetManager()->DestroyBuffer(_buffer);
}

bool Buffer::Resize(uint64_t newSize, bool bForceResize)
{
	if (_bufferCapacity >= newSize && !bForceResize)
	{
		return false;
	}

	const auto& manager = VulkanManager::GetManager();

	void* oldBufferData = _bufferMapping;
	auto oldBufferMemory = _bufferMemory;
	auto oldBuffer = _buffer;
	const auto oldBufferSize = _bufferCapacity;

	//创建新Buffer
	_bufferCapacity = newSize;
	manager->CreateBuffer(_bufferUsage, _bufferCapacity, _buffer);
	manager->AllocateBufferMemory(_buffer, _bufferMemory);

#if IS_EDITOR
	ConsoleDebug::printf_endl_succeed(GetInternationalizationText("Renderer", "ResizeBuffer"), _bufferCapacity , (double)_bufferCapacity/ (double)1024.0/ (double)1024.0);
#endif

	//把旧的缓冲区复制到新的里
	void* newBufferData = nullptr;
	auto result = vkMapMemory(manager->GetDevice(), _bufferMemory, 0, VK_WHOLE_SIZE, 0, &newBufferData);
	memcpy(newBufferData, oldBufferData, oldBufferSize);
	_bufferMapping = newBufferData;

	//关闭旧缓存映射,等待GC释放
	vkUnmapMemory(manager->GetDevice(), oldBufferMemory);
	VulkanObjectManager::Get()->SafeReleaseVkBuffer(oldBuffer, oldBufferMemory);

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

	const auto& manager = VulkanManager::GetManager();
	manager->CreateBuffer(usageFlags, bufferSize, _buffer);
	manager->AllocateBufferMemory(_buffer, _bufferMemory);

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
