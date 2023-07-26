#include "DescriptorSet.h"
#include "VulkanManager.h"

DescriptorSet::DescriptorSet(VkDescriptorType type, uint32_t bindingCount, VkShaderStageFlags shaderStageFlags)
{
	_descriptorTypes = type;
	_descriptorSetCount = 1;
	_shaderStageFlags = shaderStageFlags;
	VulkanManager::GetManager()->CreateDescripotrSetLayout(type, bindingCount, _descriptorSetLayout, _shaderStageFlags);
	if (_descriptorSetLayout != VK_NULL_HANDLE)
	{
		_descriptorSets.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			VulkanManager::GetManager()->AllocateDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), _descriptorSetLayout, 1, _descriptorSets[i]);
		}
	}
}

DescriptorSet::~DescriptorSet()
{
	//for (auto& i : _descriptorSets)
	//{
	//	  VulkanManager::GetManager()->FreeDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), i);
	//}
	VulkanManager::GetManager()->DestroyDescriptorSetLayout(_descriptorSetLayout);
	VulkanManager::GetManager()->FreeBufferMemory(_bufferMemory);
	VulkanManager::GetManager()->DestroyBuffer(_buffer);
}

void DescriptorSet::Resize(uint32_t newDescriptorSetCount)
{
	if (newDescriptorSetCount != _descriptorSetCount)
	{
		_descriptorSetCount = newDescriptorSetCount;
		//等待渲染完成
		vkDeviceWaitIdle(VulkanManager::GetManager()->GetDevice());
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			//销毁旧的DescriptorSet
			VulkanManager::GetManager()->FreeDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), _descriptorSets[i]);
			//从pool里申请新的DescriptorSet
			VulkanManager::GetManager()->AllocateDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), _descriptorSetLayout, newDescriptorSetCount, _descriptorSets[i]);
		}
	}
}

void DescriptorSet::CreateBuffer(VkBufferUsageFlags usageFlags, uint64_t bufferSize)
{
	if (_buffer != VK_NULL_HANDLE)
		return;
	_bufferCapacity = bufferSize;
	VulkanManager::GetManager()->CreateBuffer(usageFlags, bufferSize, _buffer);
	VulkanManager::GetManager()->AllocateBufferMemory(_buffer, _bufferMemory);
	MapMemory();
}

void DescriptorSet::MapMemory()
{
	if (_bufferIsMapping)
		return;
	_bufferIsMapping = true;
	auto result = vkMapMemory(VulkanManager::GetManager()->GetDevice(), _bufferMemory, 0, VK_WHOLE_SIZE, 0, &_bufferMapping);
	if (result != VK_SUCCESS)
	{
	}
}

void DescriptorSet::UnMapMemory()
{
	if (!_bufferIsMapping)
		return;
	_bufferIsMapping = false;
	vkUnmapMemory(VulkanManager::GetManager()->GetDevice(), _bufferMemory);
}
