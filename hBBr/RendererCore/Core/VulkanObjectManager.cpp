#include "VulkanObjectManager.h"
#include "VkPtr.h"

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

void VulkanObjectManager::VulkanPtrGC(VkPtrBase* vkptr)
{
	VkPtrObject newObject = { };

	newObject.ptr = vkptr->_ptr;
	newObject.typeHash = vkptr->_typeHash;
	newObject.bImmediate = (bool)vkptr->_bImmediateRelease;
	newObject.ptrCounter = vkptr->_refCounter;

	newObject.frameCount = 0;
	_vulkanPtrs.push_back(newObject);
	_numRequestObjects++;
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


	if (_vulkanPtrs.size() > 0)
	{
		for (auto it = _vulkanPtrs.begin(); it != _vulkanPtrs.end();)
		{
			auto& object = (*it);
			if (object.ptrCounter->Get() <= 0)//引用次数==0代表可以释放
			{
				if (object.frameCount > swapchainBufferCount)
				{
					if (!object.bImmediate && _gcCurrentSecond < _gcMaxSecond)
						continue;
					//正式销毁
					{
						delete object.ptrCounter;
						//根据类型针对性销毁
						if (object.typeHash == typeid(VkBuffer).hash_code())
						{
							VkBuffer buffer = (VkBuffer)object.ptr;
							manager->DestroyBuffer(buffer);
						}
						else if (object.typeHash == typeid(VkDeviceMemory).hash_code())
						{
							VkDeviceMemory deviceMemory = (VkDeviceMemory)object.ptr;
							manager->FreeBufferMemory(deviceMemory);
						}
					}
					it = _vulkanPtrs.erase(it);
					_numRequestObjects--;
					continue;
				}
				else if (object.frameCount <= swapchainBufferCount)
					object.frameCount++;
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
