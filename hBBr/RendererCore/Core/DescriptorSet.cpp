#include "DescriptorSet.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
DescriptorSet::DescriptorSet(class VulkanRenderer* renderer , VkDescriptorType type, uint32_t bindingCount, VkShaderStageFlags shaderStageFlags)
{
	_renderer = renderer;
	_descriptorTypes.push_back(type);
	_shaderStageFlags = shaderStageFlags;
	VulkanManager::GetManager()->CreateDescripotrSetLayout({ type }, _descriptorSetLayout, _shaderStageFlags);
	if (_descriptorSetLayout != VK_NULL_HANDLE)
	{
		_descriptorSets.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			VulkanManager::GetManager()->AllocateDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), _descriptorSetLayout, _descriptorSets[i]);
		}
	}
	if (type & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || type & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
		std::unique_ptr<Buffer> buffer;
		buffer.reset(new Buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
		_buffers.push_back(std::move(buffer));
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

void DescriptorSet::BufferMapping(void* mappingData, uint64_t bufferSize, int bufferIndex)
{
	auto alignmentSize =  VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSize);
	_buffers[bufferIndex]->BufferMapping(mappingData, alignmentSize);
	VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, bufferIndex, 0, alignmentSize);
}

void DescriptorSet::BufferMapping(void* mappingData, uint64_t offset, uint64_t bufferSize, int bufferIndex)
{
	auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSize);
	_buffers[bufferIndex]->BufferMapping(mappingData, offset, bufferSize);
	VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, bufferIndex, 0, alignmentSize);
}

void DescriptorSet::BufferMappingOffset(void* mappingData, uint64_t bufferSize, int bufferIndex)
{
	auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSize);
	//Get current offset
	_buffers[bufferIndex]->BufferMapping(mappingData, _currentOffset, bufferSize);
	VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, bufferIndex, 0, alignmentSize);
	//Add offset
	_currentOffset += alignmentSize;
}

const VkDescriptorSet& DescriptorSet::GetDescriptorSet()
{
	return _descriptorSets[_renderer->GetCurrentFrameIndex()];
}
