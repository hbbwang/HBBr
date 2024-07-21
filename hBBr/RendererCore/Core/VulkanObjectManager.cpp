#include "VulkanObjectManager.h"

std::unique_ptr<VulkanObjectManager> VulkanObjectManager::_vulkanObjectManager;

std::shared_ptr<VkBuffer> VulkanObjectManager::CreateVkBuffer(VkBufferUsageFlags usage, VkDeviceSize bufferSize, bool bImmediate)
{
	auto manager = VulkanManager::GetManager();

	VkBufferObject* newObject = new VkBufferObject;
	std::shared_ptr<VkBuffer> result;
	result.reset(new VkBuffer);
	manager->CreateBuffer(usage, bufferSize, *result);

	newObject->bImmediate = bImmediate;
	newObject->buffer = std::move(result);
	newObject->frameCount = 0;
	_bufferObjects.push_back(newObject);

	_numRequestObjects++;

	return newObject->buffer;
}

std::shared_ptr<VkDeviceMemory> VulkanObjectManager::AllocateVkDeviceMemory(VkBuffer buffer, VkMemoryPropertyFlags propertyFlags, bool bImmediate)
{
	auto manager = VulkanManager::GetManager();

	VkDeviceMemoryObject* newObject = new VkDeviceMemoryObject;
	std::shared_ptr<VkDeviceMemory> result;
	result.reset(new VkDeviceMemory);
	manager->AllocateBufferMemory(buffer, *result, propertyFlags);

	newObject->bImmediate = bImmediate;
	newObject->buffer = std::move(result);
	newObject->frameCount = 0;
	_deviceMemoryObjects.push_back(newObject);

	_numRequestObjects++;

	return newObject->buffer;
}

void VulkanObjectManager::Update()
{
	auto manager = VulkanManager::GetManager();
	auto swapchainBufferCount = manager->GetSwapchainBufferCount();

	if (_gcTime.IsStart())
	{
		_gcCurrentSecond = _gcTime.End_s();
	}
	else
	{
		_gcTime.Start();
		_gcCurrentSecond = _gcTime.End_s();
	}

	//VkDeviceMemory
	if (_deviceMemoryObjects.size() > 0)
	{
		for (auto it = _deviceMemoryObjects.begin(); it != _deviceMemoryObjects.end();)
		{
			auto& object = (*it);
			if (object->buffer.use_count() <= 1)//引用次数少于2代表可以释放
			{
				if (object->frameCount > swapchainBufferCount)
				{
					if (!object->bImmediate && _gcCurrentSecond < _gcMaxSecond)
						continue;
					manager->FreeBufferMemory(*object->buffer);
					it = _deviceMemoryObjects.erase(it);
					_numRequestObjects--;
					continue;
				}
				else if (object->frameCount <= swapchainBufferCount)
					object->frameCount++;
			}
			it++;
		}
	}

	//VkBuffer
	if (_bufferObjects.size() > 0)
	{
		for (auto it = _bufferObjects.begin(); it != _bufferObjects.end();)
		{
			auto& object = (*it);
			if (object->buffer.use_count() <= 1)//引用次数少于2代表可以释放
			{
				if (object->frameCount > swapchainBufferCount)
				{
					if (!object->bImmediate && _gcCurrentSecond < _gcMaxSecond)
						continue;
					manager->DestroyBuffer(*object->buffer);
					it = _bufferObjects.erase(it);
					_numRequestObjects--;
					continue;
				}
				else if(object->frameCount <= swapchainBufferCount)
					object->frameCount++;
			}	
			it++;
		}
	}

	//到时间重新计时
	if (_gcCurrentSecond > _gcMaxSecond)
	{
		_gcTime.Start();
		_gcCurrentSecond = 0;
	}
}

void VulkanObjectManager::Release()
{
	SetGCInterval(0);
	auto manager = VulkanManager::GetManager();
	manager->DeviceWaitIdle();
	//强制等待Object完全回收
	while (_numRequestObjects > 0)
	{
		Update();
	}
}
