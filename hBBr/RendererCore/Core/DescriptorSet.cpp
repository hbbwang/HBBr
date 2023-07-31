#include "DescriptorSet.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
DescriptorSet::DescriptorSet(class VulkanRenderer* renderer ,VkDescriptorType type, uint32_t bindingCount, VkShaderStageFlags shaderStageFlags)
{
	_renderer = renderer;
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
	if (type & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || type & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
		_buffer.reset(new Buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
	}
}

DescriptorSet::~DescriptorSet()
{
	//for (auto& i : _descriptorSets)
	//{
	//	  VulkanManager::GetManager()->FreeDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), i);
	//}
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	VulkanManager::GetManager()->DestroyDescriptorSetLayout(_descriptorSetLayout);
}

void DescriptorSet::Resize(uint32_t newDescriptorSetCount)
{
	if (newDescriptorSetCount != _descriptorSetCount)
	{
		_descriptorSetCount = newDescriptorSetCount;
		//等待渲染完成
		vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			//销毁旧的DescriptorSet
			VulkanManager::GetManager()->FreeDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), _descriptorSets[i]);
			//从pool里申请新的DescriptorSet
			VulkanManager::GetManager()->AllocateDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), _descriptorSetLayout, newDescriptorSetCount, _descriptorSets[i]);
		}
	}
}

std::vector<VkDescriptorSet> DescriptorSet::GetDescriptorSets() const
{
	return _descriptorSets[_renderer->GetCurrentFrameIndex()];
}
