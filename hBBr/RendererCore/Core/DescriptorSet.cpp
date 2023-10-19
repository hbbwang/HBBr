#include "DescriptorSet.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
#include <vector>
DescriptorSet::DescriptorSet(class VulkanRenderer* renderer , VkDescriptorType type, uint32_t bindingCount, VkDeviceSize bufferSizeInit, VkShaderStageFlags shaderStageFlags)
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
	auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSizeInit);
	if (type & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || type & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
		std::unique_ptr<Buffer> buffer;
		buffer.reset(new Buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, alignmentSize ));
		_buffers.push_back(std::move(buffer));
	}
	_needUpdates.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
	NeedUpdate();
}

DescriptorSet::DescriptorSet(VulkanRenderer* renderer, std::vector<VkDescriptorType> types, uint32_t bindingCount, VkDeviceSize bufferSizeInit, VkShaderStageFlags shaderStageFlags)
{
	_renderer = renderer;
	_descriptorTypes = types;
	_shaderStageFlags = shaderStageFlags;
	VulkanManager::GetManager()->CreateDescripotrSetLayout(types, _descriptorSetLayout, _shaderStageFlags);
	if (_descriptorSetLayout != VK_NULL_HANDLE)
	{
		_descriptorSets.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			VulkanManager::GetManager()->AllocateDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), _descriptorSetLayout, _descriptorSets[i]);
		}
	}
	auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSizeInit);
	for (int i = 0; i < types.size(); i++)
	{
		std::unique_ptr<Buffer> buffer;
		if (types[i] == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || types[i] == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			buffer.reset(new Buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, alignmentSize));
		}
		else if (types[i] == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
		}
		_buffers.push_back(std::move(buffer));
	}
	_needUpdates.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
	NeedUpdate();
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

void DescriptorSet::BufferMapping(void* mappingData, uint64_t offset, uint64_t bufferSize, int bufferIndex)
{
	if (_buffers[bufferIndex] == NULL)
	{
		MessageOut("Buffer Mapping Failed._buffers[bufferIndex] is NULL", false, true, "255,255,0");
		return;
	}
	_buffers[bufferIndex]->BufferMapping(mappingData, offset, bufferSize);
	//auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSize);
	//VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, bufferIndex, offset, alignmentSize);
}

bool DescriptorSet::ResizeDescriptorBuffer(VkDeviceSize newSize, int bufferIndex)
{
	if (_buffers[bufferIndex] == NULL)
	{
		MessageOut("Buffer Mapping Failed._buffers[bufferIndex] is NULL", false, true, "255,255,0");
		return false;
	}
	auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(newSize);
	if (alignmentSize > _buffers[bufferIndex]->GetBufferSize())
	{
		_buffers[bufferIndex]->Resize(alignmentSize);
		NeedUpdate();
		return true;
	}
	return false;
}

void DescriptorSet::UpdateDescriptorSet(std::vector<uint32_t> bufferRanges, std::vector<uint32_t> offsets)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, 0, bufferRanges , offsets);
	}	
}

void DescriptorSet::UpdateDescriptorSet(uint32_t bufferSize, uint32_t offset)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSize);
		VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, 0, offset, alignmentSize);
	}
}

void DescriptorSet::UpdateDescriptorSet(uint32_t sameBufferSize, std::vector<uint32_t> offsets)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sameBufferSize);
		VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, 0, (uint32_t)alignmentSize , offsets);
	}
}

void DescriptorSet::UpdateDescriptorSetAll(uint32_t sameBufferSize)
{
	auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sameBufferSize);
	VulkanManager::GetManager()->UpdateBufferDescriptorSetAll(this, 0, 0, alignmentSize);
}

const VkDescriptorSet& DescriptorSet::GetDescriptorSet()
{
	return _descriptorSets[_renderer->GetCurrentFrameIndex()];
}

const VkDescriptorSet& DescriptorSet::GetDescriptorSet(int index)
{
	return _descriptorSets[index];
}
