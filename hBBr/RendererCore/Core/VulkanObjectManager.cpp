#include "VulkanObjectManager.h"
#include "VkPtr.h"
#include "AssetObject.h"

std::unique_ptr<VulkanObjectManager> VulkanObjectManager::_vulkanObjectManager;

VulkanObjectManager::VulkanObjectManager() :
	_assetCheckCount(0),
	_maxAssetCheckCount(4),//为了提高效率，分帧检查，一帧检查4个资产，按最低标准一秒30帧，那就是1秒检查120个资产。
	_bStartCheckAsset(false),
	_gcCurrentSecond(0),
	_gcMaxSecond(40),
	_numRequestObjects(0)
{
	_vulkanPtrs.reserve(1024);
	_vulkanObjects.reserve(1024);
}

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

void VulkanObjectManager::AssetLinkGC(std::weak_ptr<class AssetObject> asset, bool bImmediate)
{
	VkAssetObject obj = {};
	obj.asset = asset;
	obj.bImmediate = bImmediate;
	_vulkanObjects.push_back(obj);
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
					delete object;
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
					delete object;
					it = _bufferObjects.erase(it);
					_numRequestObjects--;
					continue;
				}
				else if (object->frameCount <= swapchainBufferCount)
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

	if (_gcCurrentSecond >= _gcMaxSecond)
	{
		ContentManager::Get()->AssetUpdate();
	}

	//if (_vulkanObjects.size() > 0)
	//{
	//	if (_bStartCheckAsset)
	//	{
	//		const auto num = _vulkanObjects.size();
	//		auto startIndex = _assetCheckCount;
	//		for (auto i = _vulkanObjects.begin() + startIndex; i != _vulkanObjects.end(); i++)
	//		{
	//			if (!i->asset.expired())
	//			{
	//				if (!i->asset.lock()->GetResident())//常驻资产不会被GC
	//				{
	//					if (i->asset.lock().use_count() <= 1)
	//					{
	//						i->asset.lock()->_assetInfo.lock()->ReleaseData(); //卸载资产
	//					}
	//				}
	//				_assetCheckCount++;
	//				if (_assetCheckCount - startIndex >= _maxAssetCheckCount)
	//				{
	//					break;
	//				}
	//			}
	//		}
	//		if (_assetCheckCount >= num)
	//		{
	//			_assetCheckCount = 0;
	//		}
	//	}
	//}


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
