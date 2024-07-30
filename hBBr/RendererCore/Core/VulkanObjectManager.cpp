#include "VulkanObjectManager.h"
#include "AssetObject.h"

std::unique_ptr<VulkanObjectManager> VulkanObjectManager::_ptr = nullptr;

VulkanObjectManager::VulkanObjectManager() :
	_assetCheckCount(0),
	_maxAssetCheckCount(4),//为了提高效率，分帧检查，一帧检查4个资产，按最低标准一秒30帧，那就是1秒检查120个资产。
	_bStartCheckAsset(false),
	_gcCurrentSecond(0),
	_gcMaxSecond(30),
	_numRequestObjects(0)
{
	_vmaBufferObjects.reserve(20);
	_vkBufferObjects.reserve(20);
}

void VulkanObjectManager::SafeReleaseVkBuffer(VkBuffer buffer, VkDeviceMemory memory)
{
	VkBufferObject* newObject = new VkBufferObject();
	newObject->buffer = buffer;
	newObject->memory = memory;
	newObject->frameCount = 0;
	_vkBufferObjects.push_back(newObject);
	_numRequestObjects++;
}

void VulkanObjectManager::SafeReleaseVMABuffer(VkBuffer buffer, VmaAllocation allocation, HString debugName, VkDeviceSize debugSize)
{
	VMABufferObject* newObject = new VMABufferObject();
	newObject->buffer = buffer;
	newObject->allocation = allocation;
	newObject->frameCount = 0;
	newObject->debugName = debugName;
	newObject->debugSize = debugSize;
	_vmaBufferObjects.push_back(newObject);
	_numRequestObjects++;
}

void VulkanObjectManager::AssetLinkGC(std::weak_ptr<class AssetObject> asset, bool bImmediate)
{
	VkAssetObject obj = {};
	obj.asset = asset;
	obj.bImmediate = bImmediate;
	_vulkanObjects.push_back(obj);
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

	//VkBuffer
	if (_vkBufferObjects.size() > 0)
	{
		for (auto it = _vkBufferObjects.begin(); it != _vkBufferObjects.end();)
		{
			auto& object = (*it);

			if (object->frameCount > swapchainBufferCount)
			{
				manager->DestroyBuffer(object->buffer);
				manager->FreeBufferMemory(object->memory);
				delete object;
				it = _vkBufferObjects.erase(it);
				_numRequestObjects--;
				continue;
			}
			else
				object->frameCount++;
			it++;
		}
	}

	//VMA Buffer
	if (_vmaBufferObjects.size() > 0)
	{
		for (auto it = _vmaBufferObjects.begin(); it != _vmaBufferObjects.end();)
		{
			auto& object = (*it);
			
			if (object->frameCount > swapchainBufferCount)
			{
				manager->VMADestroyBufferAndFreeMemory(object->buffer, object->allocation, object->debugName, object->debugSize);
				delete object;
				it = _vmaBufferObjects.erase(it);
				_numRequestObjects--;
				continue;
			}
			else
				object->frameCount++;
			it++;
		}
	}

	//资产GC
	if (_vulkanObjects.size() > 0)
	{
		if (_bStartCheckAsset)
		{
			const auto num = _vulkanObjects.size();
			auto startIndex = _assetCheckCount;
			auto it = _vulkanObjects.begin();
			std::advance(it, startIndex); //将迭代器向前移动定位
			for (it; it != _vulkanObjects.end(); )
			{
				if (!it->asset.expired())
				{
					auto asset_ptr = it->asset.lock().get();
					if (!asset_ptr->IsResident() && !asset_ptr->IsSystemAsset())//常驻资产不会被GC
					{
						if (asset_ptr->_assetInfo.lock()->GetRefCount() <= 1)
						{
							_vulkanObjectsRelease.push_back(*it);
							it = _vulkanObjects.erase(it);
							continue;
						}
					}
					_assetCheckCount++;
					if (_assetCheckCount - startIndex >= _maxAssetCheckCount)
					{
						break;
					}
				}
				else
				{
					//无效资产,下一个
					it = _vulkanObjects.erase(it);
					_numRequestObjects--;
					continue;
				}
				it++;
			}
			if (_assetCheckCount >= num)
			{
				_assetCheckCount = 0;
				_bStartCheckAsset = false; 
			}
		}
	}
	if (_vulkanObjectsRelease.size() > 0)
	{
		for (auto it = _vulkanObjectsRelease.begin() ; it != _vulkanObjectsRelease.end(); )
		{
			if (it->frameCount > swapchainBufferCount)
			{
				auto asset_ptr = it->asset.lock().get();
				if (!asset_ptr->_assetInfo.expired())
				{
					asset_ptr->_assetInfo.lock()->ReleaseData(); //卸载资产
				}
				_numRequestObjects--;
				it = _vulkanObjectsRelease.erase(it);
				continue;
			}
			it->frameCount++;
			it++;
		}
	}


	//到时间重新计时
	if (_gcCurrentSecond > _gcMaxSecond)
	{
		_gcTime.Start();
		_gcCurrentSecond = 0;
		_bStartCheckAsset = true;
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
