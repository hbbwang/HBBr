#include "DescriptorSet.h"
#include "VulkanManager.h"
#include "VulkanRenderer.h"
#include "Texture2D.h"
#include <vector>

DescriptorSet::DescriptorSet(VulkanRenderer* renderer, VkDescriptorType type, VkDescriptorSetLayout setLayout, VkDeviceSize bufferSizeInit, VkShaderStageFlags shaderStageFlags)
{
	_renderer = renderer;
	_descriptorTypes.push_back(type);
	_shaderStageFlags.push_back(shaderStageFlags);
	if (setLayout != VK_NULL_HANDLE)
	{
		_descriptorSets.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			VulkanManager::GetManager()->AllocateDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), setLayout, _descriptorSets[i]);
		}
	}
	if (type & VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || type & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
		auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSizeInit);
		std::unique_ptr<Buffer> buffer;
		buffer.reset(new Buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, alignmentSize));
		_buffers.push_back(std::move(buffer));
	}
	_needUpdates.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
	NeedUpdate();
}

DescriptorSet::DescriptorSet(VulkanRenderer* renderer, std::vector<VkDescriptorType> types, VkDescriptorSetLayout setLayout, VkDeviceSize bufferSizeInit, std::vector<VkShaderStageFlags> shaderStageFlags)
{
	_renderer = renderer;
	_descriptorTypes = types;
	_shaderStageFlags = std::move(shaderStageFlags);
	if (setLayout != VK_NULL_HANDLE)
	{
		_descriptorSets.resize(VulkanManager::GetManager()->GetSwapchainBufferCount());
		for (int i = 0; i < (int)VulkanManager::GetManager()->GetSwapchainBufferCount(); i++)
		{
			VulkanManager::GetManager()->AllocateDescriptorSet(VulkanManager::GetManager()->GetDescriptorPool(), setLayout, _descriptorSets[i]);
		}
	}
	for (int i = 0; i < types.size(); i++)
	{
		std::unique_ptr<Buffer> buffer;
		if (types[i] == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || types[i] == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSizeInit);
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
}

void DescriptorSet::BufferMapping(void* mappingData, uint64_t offset, uint64_t bufferSize, int bufferIndex)
{
	if (_buffers[bufferIndex] == nullptr)
	{
		MessageOut("Buffer Mapping Failed._buffers[bufferIndex] is nullptr", false, true, "255,255,0");
		return;
	}
	_buffers[bufferIndex]->BufferMapping(mappingData, offset, bufferSize);
	//auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSize);
	//VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, bufferIndex, offset, alignmentSize);
}

bool DescriptorSet::ResizeDescriptorBuffer(VkDeviceSize newSize, int bufferIndex)
{
	if (_buffers[bufferIndex] == nullptr)
	{
		MessageOut("Buffer Mapping Failed._buffers[bufferIndex] is nullptr", false, true, "255,255,0");
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

void DescriptorSet::UpdateDescriptorSet(std::vector<uint32_t> bufferRanges, std::vector<uint32_t> offsets, uint32_t dstBinding)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, dstBinding, bufferRanges , offsets);
	}	
}

void DescriptorSet::UpdateDescriptorSet(uint32_t bufferSize, uint32_t offset, uint32_t dstBinding)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(bufferSize);
		VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, dstBinding, offset, alignmentSize);
	}
}

void DescriptorSet::UpdateDescriptorSet(uint32_t sameBufferSize, std::vector<uint32_t> offsets, uint32_t dstBinding)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sameBufferSize);
		VulkanManager::GetManager()->UpdateBufferDescriptorSet(this, dstBinding, (uint32_t)alignmentSize , offsets);
	}
}

void DescriptorSet::UpdateDescriptorSetAll(uint32_t sameBufferSize)
{
	auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sameBufferSize);
	VulkanManager::GetManager()->UpdateBufferDescriptorSetAll(this, 0, 0, alignmentSize);
}

void DescriptorSet::UpdateTextureDescriptorSet(std::vector<class Texture2D*> textures, std::vector<VkSampler> samplers, int beginBindingIndex)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		//
		const uint32_t count = (uint32_t)textures.size();
		std::vector<VkWriteDescriptorSet> descriptorWrite(count);
		std::vector<VkDescriptorImageInfo> imageInfo(count);
		for (uint32_t o = 0; o < count; o++)
		{
			imageInfo[o] = {};
			imageInfo[o].sampler = samplers[o];
			imageInfo[o].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[o].imageView = textures[o]->GetTextureView();
			descriptorWrite[o] = {};
			descriptorWrite[o].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[o].dstSet = this->GetDescriptorSet();
			descriptorWrite[o].dstBinding = o + beginBindingIndex;
			descriptorWrite[o].dstArrayElement = 0;
			descriptorWrite[o].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite[o].descriptorCount = 1;
			descriptorWrite[o].pBufferInfo = VK_NULL_HANDLE;
			descriptorWrite[o].pImageInfo = &imageInfo[o]; // Optional
			descriptorWrite[o].pTexelBufferView = VK_NULL_HANDLE; // Optional
		}
		vkUpdateDescriptorSets(VulkanManager::GetManager()->GetDevice(), count, descriptorWrite.data(), 0, VK_NULL_HANDLE);
	}
}

void DescriptorSet::UpdateStoreTextureDescriptorSet(std::vector<class Texture2D*> textures, int beginBindingIndex)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		//
		const uint32_t count = (uint32_t)textures.size();
		std::vector<VkWriteDescriptorSet> descriptorWrite(count);
		std::vector<VkDescriptorImageInfo> imageInfo(count);
		for (uint32_t o = 0; o < count; o++)
		{
			imageInfo[o] = {};
			imageInfo[o].sampler = nullptr;
			imageInfo[o].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo[o].imageView = textures[o]->GetTextureView();
			descriptorWrite[o] = {};
			descriptorWrite[o].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[o].dstSet = this->GetDescriptorSet();
			descriptorWrite[o].dstBinding = o + beginBindingIndex;
			descriptorWrite[o].dstArrayElement = 0;
			descriptorWrite[o].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrite[o].descriptorCount = 1;
			descriptorWrite[o].pBufferInfo = VK_NULL_HANDLE;
			descriptorWrite[o].pImageInfo = &imageInfo[o]; // Optional
			descriptorWrite[o].pTexelBufferView = VK_NULL_HANDLE; // Optional
		}
		vkUpdateDescriptorSets(VulkanManager::GetManager()->GetDevice(), count, descriptorWrite.data(), 0, VK_NULL_HANDLE);
	}
}

void DescriptorSet::UpdateTextureViewDescriptorSet(std::vector<VkImageView> images, std::vector<VkSampler> samplers)
{
	if (_needUpdates[_renderer->GetCurrentFrameIndex()] == 1)
	{
		_needUpdates[_renderer->GetCurrentFrameIndex()] = 0;
		//
		const uint32_t count = (uint32_t)images.size();
		std::vector<VkWriteDescriptorSet> descriptorWrite(count);
		std::vector<VkDescriptorImageInfo> imageInfo(count);
		for (uint32_t o = 0; o < count; o++)
		{
			imageInfo[o] = {};
			imageInfo[o].sampler = samplers[o];
			imageInfo[o].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[o].imageView = images[o];
			descriptorWrite[o] = {};
			descriptorWrite[o].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[o].dstSet = this->GetDescriptorSet();
			descriptorWrite[o].dstBinding = o;
			descriptorWrite[o].dstArrayElement = 0;
			descriptorWrite[o].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite[o].descriptorCount = 1;
			descriptorWrite[o].pBufferInfo = VK_NULL_HANDLE;
			descriptorWrite[o].pImageInfo = &imageInfo[o]; // Optional
			descriptorWrite[o].pTexelBufferView = VK_NULL_HANDLE; // Optional
		}
		vkUpdateDescriptorSets(VulkanManager::GetManager()->GetDevice(), count, descriptorWrite.data(), 0, VK_NULL_HANDLE);
	}
}

const VkDescriptorSet& DescriptorSet::GetDescriptorSet()
{
	return _descriptorSets[_renderer->GetCurrentFrameIndex()];
}

const VkDescriptorSet& DescriptorSet::GetDescriptorSet(int index)
{
	return _descriptorSets[index];
}
